// Fill out your copyright notice in the Description page of Project Settings.

#include "MontageGraphComponent.h"

#include "GameplayPrediction.h"
#include "MontageGraph.h"
#include "MontageGraphDelegates.h"
#include "Nodes/MGNode.h"
#include "Nodes/MGNode_Montage.h"

#include "AbilitySystemComponent.h"
#include "DisplayDebugHelpers.h"
#include "KismetTraceUtils.h"
#include "MontageGraphReplication.h"
#include "MontageGraphTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayAnimAndWait.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "Gameplay/MontageGraphGameplayEffectContext.h"
#include "Net/UnrealNetwork.h"
#include "Nodes/MGEdge.h"
#include "Tracers/MontageCollisionTracer.h"

DEFINE_LOG_CATEGORY(LogMontageGraph);

DECLARE_STATS_GROUP(TEXT("MontageGraphComponent"), STATGROUP_MontageGraph, STATCAT_Advanced);


DECLARE_CYCLE_STAT(TEXT("MontageGraph - Graph Evaluated"), STAT_ActionGraphEval, STATGROUP_MontageGraph);
DECLARE_CYCLE_STAT(TEXT("MontageGraph - CollisionTick"), STAT_MontageGraphCollisionTick, STATGROUP_MontageGraph);

namespace MontageGraphCVars
{

	static float            TrustedPredictionTimeWindow = .15f;
	FAutoConsoleVariableRef CVarTrustedClientTimeframe(
		TEXT("MontageGraph.TrustedPredictionTimeWindow"),
		TrustedPredictionTimeWindow,
		TEXT("If the server recieves a prediction within this time frame, we will fast forward the server/replicated montage to match the prediction."),
		ECVF_Default);

	static float            MaxPredictionDelta = .3f;
	FAutoConsoleVariableRef CVarMaxPredictionTimeframe(
		TEXT("MontageGraph.MaxPredictionDelta"),
		MaxPredictionDelta,
		TEXT(
			"If the server recieves the link request after this timeframe, the link and prediction is automatically rejected"),
		ECVF_Default);

	static int32            bShowCollisionTracers = false;
	FAutoConsoleVariableRef CVarShowMontageGraphDebug(
		TEXT("MontageGraph.ShowCollisionTracers"),
		bShowCollisionTracers,
		TEXT("Show MontageGraph Debug information"),
		ECVF_Cheat);

	static float            IntersectionDebugHitTime = 2.f;
	FAutoConsoleVariableRef CVarIntersectionDebugHitTime(
		TEXT("MontageGraph.IntersectionDebugHitTime "),
		IntersectionDebugHitTime,
		TEXT("How long to show Intersection hit points"),
		ECVF_Cheat);

	static int32            ExtraCollisionIterations = 1;
	FAutoConsoleVariableRef CVarExtraCollisionIterations(
		TEXT("MontageGraph.MaxCollisionIterations "),
		ExtraCollisionIterations,
		TEXT("How many sub-frame sweep trace iterations should the collision check perform. Used to increase the precision of the collision.  Ex: if we're at fram x, and max iterations is set to 4(default) it will check the frame range [x, x+3], at each frame."),
		ECVF_Default);

	static int32            MaxQueueSize = 1;
	FAutoConsoleVariableRef CVarMaxQueueSize(
		TEXT("MontageGraph.MaxQueueSize "),
		MaxQueueSize,
		TEXT("How many nodes are we allowed to queue while playing a montage? Usually 1, as we want to be able to link the next attack combo"),
		ECVF_Default);

}

bool FMGReplicatedLinkInfo::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << NodeID;
	Ar << SyncTime;

	bOutSuccess = true;
	return true;
}

// Sets default values for this component's properties
UMontageGraphComponent::UMontageGraphComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	SweepTickFunc.bCanEverTick          = true;
	SweepTickFunc.bStartWithTickEnabled = false;
	SweepTickFunc.SetTickFunctionEnable(false);
	SweepTickFunc.TickGroup                   = TG_PostPhysics;
	SweepTickFunc.bRunOnAnyThread             = false;
	SweepTickFunc.bAllowTickOnDedicatedServer = true;
	
	
	SetIsReplicatedByDefault(true);
}

void UMontageGraphComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	// If the owner ticks, make sure we tick first. This is to ensure the owner's location will be up to date when it ticks.
	AActor* Owner = GetOwner();
	if (bRegister && PrimaryComponentTick.bCanEverTick && Owner && Owner->CanEverTick())
	{
		Owner->PrimaryActorTick.AddPrerequisite(this, PrimaryComponentTick);
	}

	if (bRegister)
	{
		if (SetupActorComponentTickFunction(&SweepTickFunc))
		{
			SweepTickFunc.AddPrerequisite(this, this->PrimaryComponentTick);
			SweepTickFunc.Target = this;
		}
	}
	else
	{
		if (SweepTickFunc.IsTickFunctionRegistered())
		{
			SweepTickFunc.UnRegisterTickFunction();
		}
	}
}

FTransform UMontageGraphComponent::GetTransformForCollisionTracer(UCollisionTracer* CollisionData)
{
	if (UAnimInstance* AnimInstance = GetAnimInstance())
	{
		if (USkeletalMeshComponent* SkelMesh = AnimInstance->GetSkelMeshComponent())
		{
			return SkelMesh->GetComponentTransform();
		}
	}
	return FTransform::Identity;
}

void UMontageGraphComponent::ExecuteMontageCollisionTraceTick(float AnimAlpha)
{
	SCOPE_CYCLE_COUNTER(STAT_MontageGraphCollisionTick);

	UCollisionTracer* CollisionData = ActiveLink.TracerPtr;
	if (!CollisionData || CollisionData->SamplePositions.Num() == 0)
	{
		return;
	}

	const FTransform BaseTransform = GetTransformForCollisionTracer(CollisionData);
	const FVector Extent = CollisionData->CollisionExtent * 0.5f;
	const int32 CurrentFrame = CollisionData->GetCurrentFrame(AnimAlpha);
	const int32 NumFrames = ActiveLink.ValidatedFrameIndices.Num();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(UMontageGraphComponent), false);
	Params.AddIgnoredActors(TraceActorsToIgnore);
	Params.bReturnPhysicalMaterial = true;

	// Reuse member array to avoid per-tick allocation
	FramesToValidate.Reset();
	for (int32 FrameIndex = ActiveLink.LastValidatedFrame; FrameIndex < NumFrames; ++FrameIndex)
	{
		if (FrameIndex < CurrentFrame && !ActiveLink.IsFrameValidated(FrameIndex))
		{
			FramesToValidate.Add(FrameIndex);
		}
	}

	// Reuse pooled hit results
	PooledHitResults.Reset();

	for (const int32 FrameIndex : FramesToValidate)
	{
		for (int32 i = 0; i < MontageGraphCVars::ExtraCollisionIterations; ++i)
		{
			const int32 SubSampleIndexStart = FMath::Max(FrameIndex - i, 0);
			const int32 SubSampleIndexEnd = FMath::Max(FrameIndex + 1 - i, 0);

			const FVector Start = BaseTransform.TransformPosition(CollisionData->SamplePositions[SubSampleIndexStart]);
			const FVector End = BaseTransform.TransformPosition(CollisionData->SamplePositions[SubSampleIndexEnd]);
			const FQuat Orientation = BaseTransform.TransformRotation(CollisionData->SampleOrientations[SubSampleIndexStart]);

			const FCollisionShape CollisionShape = FCollisionShape::MakeBox(Extent);

			PooledHitResults.Reset();
			const bool bHit = GetWorld()->SweepMultiByChannel(
				PooledHitResults,
				Start,
				End,
				Orientation,
				CollisionData->TraceChannel,
				CollisionShape,
				Params);

			if (bHit)
			{
				for (const FHitResult& Hit : PooledHitResults)
				{
					const AActor* HitActor = Hit.GetActor();
					if (!HitActor)
					{
						continue;
					}

					// O(1) lookup using TMap instead of O(n) FindByPredicate
					FCollisionSweepIntersection* Intersection = ActorIntersectionMap.Find(HitActor);

					if (!Intersection)
					{
						FCollisionSweepIntersection& NewIntersection = ActorIntersectionMap.Add(HitActor);
						NewIntersection.Actor = HitActor;
						NewIntersection.NumHits = 0;

						NotifyBeginIntersection(NewIntersection, Hit);

						Intersection = &NewIntersection;
					}
					else
					{
						NotifyIntersectionUpdated(*Intersection, Hit);
					}
					Intersection->NumHits++;

#if !UE_BUILD_SHIPPING
					if (MontageGraphCVars::bShowCollisionTracers)
					{
						DrawDebugBoxTraceSingle(GetWorld(), Start, End, Extent, Orientation.Rotator(), EDrawDebugTrace::ForOneFrame, bHit, Hit, FColor::Silver, FColor::Orange, 0.f);

						if (MontageGraphCVars::IntersectionDebugHitTime > 0.f)
						{
							DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 25.f, FColor::Red, false, MontageGraphCVars::IntersectionDebugHitTime, 1);
						}
					}
					DebugHitCollisionIndices.AddUnique(FrameIndex);
#endif
				}
			}
		}
		ActiveLink.SetFrameValidated(FrameIndex, true);
		ActiveLink.LastValidatedFrame = FMath::Max(FrameIndex, ActiveLink.LastValidatedFrame);
	}

#if !UE_BUILD_SHIPPING
	if (MontageGraphCVars::bShowCollisionTracers)
	{
		for (int32 j = 0; j < NumFrames; j++)
		{
			const FColor DefaultColor = ActiveLink.IsFrameValidated(j) ? FColor(40, 186, 30, 50) : FColor(94, 118, 100, 50);
			const FColor HitColor = FColor(255, 0, 15, 100);

			const FVector DebugStart = BaseTransform.TransformPosition(CollisionData->SamplePositions[j]);
			const FQuat DebugOrientation = BaseTransform.TransformRotation(CollisionData->SampleOrientations[j]);

			DrawDebugBox(GetWorld(), DebugStart, Extent, DebugOrientation,
				DebugHitCollisionIndices.Contains(j) ? HitColor : DefaultColor, false, 0.0f, 1, 0.75f);
		}
	}
#endif
}

void UMontageGraphComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Fast Arrays don't use push model, but there's no harm in marking them with it.
	// The flag will just be ignored.
	FDoRepLifetimeParams Params;
	Params.Condition    = COND_SkipOwner;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UMontageGraphComponent, RepLinkInfo, Params);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UMontageGraphComponent::Init(UAbilitySystemComponent* InASC)
{
	ASC = InASC;

	// Cache the AnimInstance for faster access
	if (ASC && ASC->AbilityActorInfo.IsValid())
	{
		CachedAnimInstance = ASC->AbilityActorInfo->GetAnimInstance();
	}
	else
	{
		CachedAnimInstance = nullptr;
	}

	MontageEndedDelegate.BindUObject(this, &ThisClass::OnMontageEnded);
	BlendingOutDelegate.BindUObject(this, &ThisClass::OnMontageBlendingOut);

	SetState(EMontageGraphState::Idle);

#if WITH_EDITOR
	FMontageGraphDelegates::OnGraphInitialized.Broadcast(this);
#endif
}

void UMontageGraphComponent::SetState(EMontageGraphState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
	}
}

void UMontageGraphComponent::SetTraceActorsToIgnore(TArray<AActor*> ActorsToIgnore)
{
	TraceActorsToIgnore = ActorsToIgnore;
}

void UMontageGraphComponent::SelectNode(UMGNode* NodeToTransitionTo)
{
	SelectedNode = NodeToTransitionTo;
	SetState(EMontageGraphState::NodeSelected);
	NotifyNodeSelected();
}

UMGNode* UMontageGraphComponent::FindNode(const FGameplayTag& BindTag,
	const FGameplayTagContainer& AdditionalTags)
{
	if (!Graph || !ASC || !Graph->RootNodes.Contains(BindTag))
	{
		return nullptr;
	}

	FGameplayTagContainer TagsToMatch = ASC->GetOwnedGameplayTags();
	TagsToMatch.AppendTags(AdditionalTags);

	UMGNode* QueryNode = Graph->RootNodes[BindTag];
	UMGNode* NodeToTransitionTo = QueryNode;

	while (QueryNode)
	{
		// Find the first valid transition
		for (const UMGEdge* NodeEdge : QueryNode->Edges)
		{
			if (!NodeEdge || !NodeEdge->EndNode)
			{
				continue;
			}

			if (const UMGEdge_TagQuery* TagQueryEdge = Cast<UMGEdge_TagQuery>(NodeEdge))
			{
				if (TagQueryEdge->Query.Matches(TagsToMatch))
				{
					NodeToTransitionTo = NodeEdge->EndNode;
					break;
				}
			}
			else if (const UMGEdge_StackCount* StackCountEdge = Cast<UMGEdge_StackCount>(NodeEdge))
			{
				const int32 StackCount = ASC->GetAggregatedStackCount(StackCountEdge->Query);
				if (StackCount >= StackCountEdge->RequiredNumOfStacks)
				{
					NodeToTransitionTo = NodeEdge->EndNode;
					break;
				}
			}
		}

		if (QueryNode->Edges.IsEmpty() || QueryNode == NodeToTransitionTo)
		{
			break;
		}

		QueryNode = NodeToTransitionTo;
	}

	if (NodeToTransitionTo)
	{
		SelectNode(NodeToTransitionTo);
	}

	return SelectedNode;
}

void UMontageGraphComponent::SetGraph(UMontageGraph* InGraph)
{
	Graph = InGraph;

	// UMontageGraphGlobals::Get().PrecacheGraph(Graph);
}

const UMontageGraph* UMontageGraphComponent::GetGraph() const
{
	return Graph;
}

void UMontageGraphComponent::ResetActiveLink()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.ClearTimer(QueuedLinkTimerHandle);
		TimerManager.ClearTimer(StartCollisionTimerHandle);

		if (TimerManager.IsTimerActive(StopCollisionTimerHandle))
		{
			TimerManager.ClearTimer(StopCollisionTimerHandle);
			if (StopCollisionSweepDelegate.IsBound())
			{
				StopCollisionSweepDelegate.Execute();
			}
		}
	}

	StopLinkedMontageTick();

	ActiveLink.Reset();

	SetState(EMontageGraphState::Idle);
}

void UMontageGraphComponent::ActivateMontageLink(UMGNode* NodeToLink, UAnimMontage* MontageToPlay)
{
	if (!Graph || !NodeToLink || !MontageToPlay)
	{
		UE_LOG(LogMontageGraph, Warning, TEXT("[%s]: ActivateMontageLink called with invalid parameters"),
			*GetFullNameSafe(this));
		return;
	}

	// Validate NodeToLink->ID is within bounds
	if (!Graph->CollisionTracers.IsValidIndex(NodeToLink->ID))
	{
		UE_LOG(LogMontageGraph, Warning, TEXT("[%s]: NodeID %d is out of bounds for CollisionTracers array"),
			*GetFullNameSafe(this), NodeToLink->ID);
		return;
	}

	StartLinkedMontageSweep(MontageToPlay);

	UCollisionTracer* CollisionDataPtr = Graph->CollisionTracers[NodeToLink->ID];
	if (CollisionDataPtr)
	{
		const double MontageLength = MontageToPlay->GetPlayLength();
		const double SweepStartTime = CollisionDataPtr->AnimSampleRange.AnimStartAlpha * MontageLength;
		const double SweepEndTime = CollisionDataPtr->AnimSampleRange.AnimEndAlpha * MontageLength;

		if (UWorld* World = GetWorld())
		{
			FTimerManager& TimerManager = World->GetTimerManager();
			TimerManager.SetTimer(StartCollisionTimerHandle, StartCollisionSweepDelegate, SweepStartTime, false);
			TimerManager.SetTimer(StopCollisionTimerHandle, StopCollisionSweepDelegate, SweepEndTime, false);
		}

		ActiveLink.InitializeCollision(CollisionDataPtr);
	}

	ActiveLink.EffectContextHandle = FGameplayEffectContextHandle(new FMontageGraphGameplayEffectContext());
	ActiveLink.LinkedNode = NodeToLink;
	ActiveLink.Montage = MontageToPlay;

	SetState(EMontageGraphState::LinkActive);
	NotifyNodeLinked(NodeToLink);

	
#if WITH_EDITOR
	FMontageGraphDelegates::OnGraphNodeLinked.Broadcast(this, *SelectedNode);
#endif
}

void UMontageGraphComponent::OnMontageEnded(UAnimMontage* AnimMontage, bool bInterrupted)
{
	if (ActiveLink.Montage == AnimMontage)
	{
		ResetActiveLink();
		
		if (!bInterrupted)
		{
			LinkQueuedNode();
		}
	}
}

void UMontageGraphComponent::OnMontageBlendingOut(UAnimMontage* AnimMontage, bool bInterrupted)
{
	if (ActiveLink.Montage == AnimMontage)
	{
		SetState(EMontageGraphState::BlendingOut);

		if (bInterrupted)
		{
			QueuedNode = nullptr;
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(QueuedLinkTimerHandle);
			}
		}

		NotifyLinkBlendingOut(ActiveLink.LinkedNode);

		StopLinkedMontageTick();
	}
}

UAnimInstance* UMontageGraphComponent::GetAnimInstance() const
{
	// Try cached value first for performance
	if (CachedAnimInstance.IsValid())
	{
		return CachedAnimInstance.Get();
	}

	// Fallback to ASC lookup if cache is stale
	if (ASC && ASC->AbilityActorInfo.IsValid())
	{
		return ASC->AbilityActorInfo->GetAnimInstance();
	}

	return nullptr;
}

void UMontageGraphComponent::BeginPlay()
{
	Super::BeginPlay();

	// Use weak pointer captures to prevent crashes if component is destroyed while timer is pending
	TWeakObjectPtr<UMontageGraphComponent> WeakThis(this);

	StartCollisionSweepDelegate = FTimerDelegate::CreateLambda([WeakThis]() {
		if (UMontageGraphComponent* Self = WeakThis.Get())
		{
			Self->NotifyCollisionSweepStarted();
		}
	});

	StopCollisionSweepDelegate = FTimerDelegate::CreateLambda([WeakThis]() {
		if (UMontageGraphComponent* Self = WeakThis.Get())
		{
			Self->NotifyCollisionSweepEnded();
		}
	});

	QueuedLinkDelegate = FTimerDelegate::CreateLambda([WeakThis]() {
		if (UMontageGraphComponent* Self = WeakThis.Get())
		{
			Self->LinkQueuedNode();
		}
	});
}

void UMontageGraphComponent::OnRep_ReplicatedAnimMontage()
{
	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		// Validate NodeID before use
		UAnimMontage* MontageToPlay = GetMontageForNodeID(RepLinkInfo.NodeID);
		if (MontageToPlay)
		{
			AnimInstance->Montage_Play(MontageToPlay, RepLinkInfo.PlayRate);
		}
		else
		{
			UE_LOG(LogMontageGraph, Warning, TEXT("[%s]: OnRep_ReplicatedAnimMontage - Invalid NodeID %d"),
				*GetFullNameSafe(this), RepLinkInfo.NodeID);
		}
	}
}

void UMontageGraphComponent::StartLinkedMontageSweep(const UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

#if !UE_BUILD_SHIPPING
	DebugHitCollisionIndices.Reset();
#endif

	SweepTickFunc.StartTime = ActiveLink.StartTime;
	SweepTickFunc.EndTime = ActiveLink.StartTime + Montage->GetPlayLength();
	SweepTickFunc.AccumulatedDilatedTime = 0.f;
	SweepTickFunc.SetTickFunctionEnable(true);
	SweepTickFunc.TickInterval = Montage->GetSamplingFrameRate().AsInterval();

	// Clear intersection map for new sweep
	ActorIntersectionMap.Reset();
}

void UMontageGraphComponent::StopLinkedMontageTick()
{
	ActorIntersectionMap.Reset();

	SweepTickFunc.SetTickFunctionEnable(false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StopCollisionTimerHandle);
	}
}

void UMontageGraphComponent::StopLinkedMontage(float BlendTimeOverride)
{
	ResetActiveLink();
}

bool UMontageGraphComponent::CanEnqueueNode() const
{
	if (!ASC || !ASC->AbilityActorInfo)
	{
		UE_LOG(LogMontageGraph, Error, TEXT("[%s]: Attempted to link Node with no valid Ability System"),
			*GetFullNameSafe(this));

		return false;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogMontageGraph, Error, TEXT("[%s]: Attempted to play MontageNode with no AnimInstance"),
			*GetFullNameSafe(this));
		return false;
	}

	return true;
}

void UMontageGraphComponent::ServerQueueNodeLink_Implementation(uint16 NodeID)
{
	if (!Graph)
	{
		UE_LOG(LogMontageGraph, Error, TEXT("[%s]: ServerQueueNodeLink called with no valid Graph"),
			*GetFullNameSafe(this));
		return;
	}

	if (!Graph->MontageNodes.IsValidIndex(NodeID))
	{
		UE_LOG(LogMontageGraph, Error,
			TEXT("[%s]: Attempted to queue NodeID[%i] with no valid Node, this shouldn't happen"),
			*GetFullNameSafe(this),
			NodeID);
		return;
	}

	QueueNodeLink(Graph->MontageNodes[NodeID]);
}

void UMontageGraphComponent::QueueNodeLink(UMGNode* NodeToLink, bool bNotifyServer)
{
	if (!CanEnqueueNode())
	{
		return;
	}

	if (ActiveLink.LinkedNode)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(QueuedLinkTimerHandle);
		}

		if (UMGNode_Montage* ActiveMontageNode = Cast<UMGNode_Montage>(ActiveLink.LinkedNode))
		{
			QueuedNode = NodeToLink;
			SetState(EMontageGraphState::NodeQueued);

			if (ActiveMontageNode->BlendLinks.Contains(NodeToLink))
			{
				const float LocalTime = MontageGraphNetwork::GetServerTime(this);
				const float TimeSinceActiveLinkStart = LocalTime - ActiveLink.StartTime;

				FMontageGraphLinkSettings& Link = ActiveMontageNode->BlendLinks[NodeToLink];
				// Copy blend settings to avoid dangling pointer
				ActiveLink.BlendSettings = Link.BlendSettings;
				ActiveLink.TargetLinkToTime = Link.TargetLinkToTime;

				if (TimeSinceActiveLinkStart > Link.StartTime)
				{
					LinkQueuedNode();
					return;
				}

				const float TimeToQueuedLink = Link.StartTime - TimeSinceActiveLinkStart;
				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().SetTimer(QueuedLinkTimerHandle, QueuedLinkDelegate, TimeToQueuedLink, false);
				}
			}
		}
	}
	else
	{
		LinkMontageNode(NodeToLink, IsPredicting());
	}
}

void UMontageGraphComponent::LinkQueuedNode()
{
	if (QueuedNode)
	{
		LinkMontageNode(QueuedNode, IsPredicting());
		QueuedNode = nullptr;
	}
}

void UMontageGraphComponent::ServerLinkNode_Implementation(
	uint16 NodeIDToLink,
	float PredictionStartTime,
	FPredictionKey PredictionKey)
{
	if (!Graph || !Graph->MontageNodes.IsValidIndex(NodeIDToLink))
	{
		UE_LOG(LogMontageGraph, Error, TEXT("[%s]: ServerLinkNode called with invalid Graph or NodeID %d"),
			*GetFullNameSafe(this), NodeIDToLink);
		ClientPredictedLinkRejected(NodeIDToLink);
		return;
	}

	const float LocalTime = MontageGraphNetwork::GetServerTime(this); // Time at which the Server got the prediction info
	const float PredictionDelta = LocalTime - PredictionStartTime;    // Time difference between Prediction/Server

	// Reject the predicted montage if its bigger than the allowed delta
	if (PredictionDelta > MontageGraphCVars::MaxPredictionDelta)
	{
		ClientPredictedLinkRejected(NodeIDToLink);
		return;
	}

	LinkMontageNode(Graph->MontageNodes[NodeIDToLink], false);
}

void UMontageGraphComponent::ClientPredictedLinkRejected_Implementation(uint16 RejectedNodeID)
{
	OnPredictiveLinkRejected(RejectedNodeID);
}

void UMontageGraphComponent::ClientPredictedLinkAccepted_Implementation(uint16 AcceptedNodeID)
{
}

void UMontageGraphComponent::LinkMontageNode(UMGNode* NodeToLink, const bool bIsPredicting)
{
	if (!NodeToLink)
	{
		return;
	}

	const uint16 NodeID = NodeToLink->ID;
	UAnimMontage* MontageToPlay = GetMontageForNodeID(NodeID);
	if (!MontageToPlay)
	{
		UE_LOG(LogMontageGraph, Error,
			TEXT("[%s]: Attempted to link Node with no valid Montage, have the montages been rebuilt?"),
			*GetFullNameSafe(this));
		return;
	}

	UAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogMontageGraph, Error,
			TEXT("[%s]: Attempted to link Node with no valid AnimInstance"),
			*GetFullNameSafe(this));
		return;
	}

	ActiveLink.StartTime = MontageGraphNetwork::GetServerTime(this);

	// Inform the server of the predictive link
	if (bIsPredicting)
	{
		// ServerLinkNode(NodeID, LocalTime, ASC->ScopedPredictionKey);
	}
	else
	{
		const float MontageLength = MontageToPlay->GetPlayLength();
		RepLinkInfo.SyncTime = ActiveLink.StartTime + MontageLength;
		RepLinkInfo.NodeID = NodeID;
		MARK_PROPERTY_DIRTY_FROM_NAME(UMontageGraphComponent, RepLinkInfo, this);
	}

	// Use TOptional's IsSet() to check if blend settings are valid
	if (ActiveLink.BlendSettings.IsSet())
	{
		AnimInstance->Montage_PlayWithBlendSettings(MontageToPlay, ActiveLink.BlendSettings.GetValue(), 1.f, EMontagePlayReturnType::Duration, ActiveLink.TargetLinkToTime);
	}
	else
	{
		AnimInstance->Montage_Play(MontageToPlay, 1.f, EMontagePlayReturnType::Duration, ActiveLink.TargetLinkToTime);
	}

	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

	ActivateMontageLink(NodeToLink, MontageToPlay);
}

void UMontageGraphComponent::CancelNodeLink()
{
	OnNodeLinkFailed.Broadcast(SelectedNode);
}

void UMontageGraphComponent::OnPredictiveLinkRejected(uint16 NodeID)
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

	UAnimMontage* MontageToPlay = GetMontageForNodeID(NodeID);
	UAnimInstance* AnimInstance = GetAnimInstance();

	if (AnimInstance && MontageToPlay && AnimInstance->Montage_IsPlaying(MontageToPlay))
	{
		AnimInstance->Montage_Stop(MONTAGE_PREDICTION_REJECT_FADETIME, MontageToPlay);
	}

	SetState(EMontageGraphState::Idle);

#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("LINK REJECTED"));
	}
#endif
}

UAnimMontage* UMontageGraphComponent::GetMontageForNodeID(uint16 NodeID) const
{
	if (!IsValid(Graph))
	{
		UE_LOG(LogMontageGraph, Error, TEXT("[%s]: GetMontageForNodeID called with invalid Graph"),
			*GetFullNameSafe(this));
		return nullptr;
	}

	if (!Graph->Montages.IsValidIndex(NodeID))
	{
		UE_LOG(LogMontageGraph, Warning, TEXT("[%s]: NodeID %d is out of bounds for Montages array (size: %d)"),
			*GetFullNameSafe(this), NodeID, Graph->Montages.Num());
		return nullptr;
	}

	return Graph->Montages[NodeID];
}

void UMontageGraphComponent::NotifyLinkBlendingOut(UMGNode* LinkedNode)
{
	// Base implementation - derived classes may override
}

void UMontageGraphComponent::NotifyNodeSelected()
{
	OnNodeSelected.Broadcast(SelectedNode);
#if WITH_EDITOR
	FMontageGraphDelegates::OnGraphNodeSelected.Broadcast(this, *SelectedNode);
#endif
}

void UMontageGraphComponent::NotifyCollisionSweepStarted()
{
	OnSweepStateChanged.Broadcast(true);
}

void UMontageGraphComponent::NotifyCollisionSweepEnded()
{
	OnSweepStateChanged.Broadcast(false);
}

bool UMontageGraphComponent::IsPredicting() const
{
	if (!ASC || !ASC->AbilityActorInfo.IsValid())
	{
		return false;
	}

	const bool bIsLocallyControlled = ASC->AbilityActorInfo->IsLocallyControlled();
	const bool bIsAuthority = ASC->AbilityActorInfo->IsNetAuthority();

	// LocalPredicted and ServerInitiated are both valid because in both those modes the ability also runs on the client
	return (!bIsAuthority && bIsLocallyControlled);
}

void UMontageGraphComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	if (DisplayInfo.IsDisplayOn(TEXT("MontageGraph")))
	{
		UMontageGraphComponent* MGC = nullptr;
		if (AActor* Actor = HUD->GetCurrentDebugTargetActor())
		{
			MGC = Actor->FindComponentByClass<UMontageGraphComponent>();
		}

		if (MGC)
		{
			TArray<FName>     LocalDisplayNames;
			FDebugDisplayInfo LocalDisplayInfo(LocalDisplayNames, TArray<FName>());
			MGC->DisplayDebug(Canvas, LocalDisplayInfo, YL, YPos);
		}
	}
}

void UMontageGraphComponent::DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	if (!Canvas || !GEngine)
	{
		return;
	}

	FFontRenderInfo RenderInfo = FFontRenderInfo();
	RenderInfo.bEnableShadow = true;

	UFont* Font = GEngine->GetMediumFont();
	if (!Font)
	{
		return;
	}

	// Display current state
	Canvas->SetDrawColor(FColor::White);
	const TCHAR* StateNames[] = { TEXT("Idle"), TEXT("NodeSelected"), TEXT("NodeQueued"), TEXT("LinkActive"), TEXT("BlendingOut") };
	const FString StateText = FString::Printf(TEXT("State: %s"), StateNames[static_cast<uint8>(CurrentState)]);
	YPos += 10.f;
	Canvas->DrawText(Font, StateText, 4.f, YPos, 1.5f, 1.5f, RenderInfo);

	if (QueuedNode)
	{
		Canvas->SetDrawColor(FColor::Blue);
		YPos += 10.f;
		Canvas->DrawText(Font, *FString::Printf(TEXT("[%s] ID[%i]"), *QueuedNode->GetNodeTitle().ToString(), QueuedNode->ID), 4.f, YPos + 10.f, 1.5f, 1.5f, RenderInfo);
	}

	Canvas->SetDrawColor(FColor::Orange);
	for (const auto& Pair : ActorIntersectionMap)
	{
		const FCollisionSweepIntersection& Intersection = Pair.Value;
		if (Intersection.Actor)
		{
			const FString TextLine = FString::Printf(TEXT("[%s]: hit %i times"), *Intersection.Actor->GetName(), Intersection.NumHits);
			const int32 LineHeight = Font->GetStringHeightSize(*TextLine) + 2.f;

			YPos += LineHeight;
			Canvas->DrawText(Font, TextLine, 8.f, YPos, 1.5f, 1.5f, RenderInfo);
		}
	}
}