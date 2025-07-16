// Fill out your copyright notice in the Description page of Project Settings.


#include "MontageGraphComponent.h"


#include "MontageGraph.h"
#include "MontageGraphDelegates.h"
#include "Nodes/MGNode.h"
#include "Nodes/MGNode_Montage.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Nodes/MGEdge.h"
#include "Tracers/MontageCollisionTracer.h"


DECLARE_STATS_GROUP(TEXT("MontageGraphComponent"), STATGROUP_MONTAGEGRAPH, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("MontageGraph - Graph Evaluated"), STAT_ActionGraphEval, STATGROUP_MONTAGEGRAPH);

namespace MontageGraphCVars
{
	static float LinkClientTrustedTimeframe = .3f;
	FAutoConsoleVariableRef CVarTrustedClientTimeframe(
		TEXT("MontageGraph.LinkClientTrustedTimeframe"),
		LinkClientTrustedTimeframe,
		TEXT("How long each round should be in seconds"),
		ECVF_Default);


	static float MaxPredictionTimeframe = 1.5f;
	FAutoConsoleVariableRef CVarMaxPredictionTimeframe(
		TEXT("MontageGraph.MaxPredictionTimeframe"),
		MaxPredictionTimeframe,
		TEXT(
			"If the server recieves the link request after this timeframe, the link and prediction is automatically rejected"),
		ECVF_Default);


	static int32 bShowCollisionTracers = false;
	FAutoConsoleVariableRef CVarShowMontageGraphDebug(
		TEXT("MontageGraph.ShowCollisionTracers"),
		bShowCollisionTracers,
		TEXT("Show MontageGraph Debug information"),
		ECVF_Default);

	
	static int32 bDoPredictiveRollback = false;
	FAutoConsoleVariableRef CVarDoPredictiveRollback(
		TEXT("MontageGraph.DoPredictiveRollback"),
		bDoPredictiveRollback ,
		TEXT("Whether the system should save player positions and check in the past for a hit collision"),
		ECVF_Default);
}


bool FMontageGraphRepInfo::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
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
	SetIsReplicatedByDefault(true);


	CollisionTracerTickFunction.bCanEverTick = true;
	CollisionTracerTickFunction.bStartWithTickEnabled = false;
	CollisionTracerTickFunction.SetTickFunctionEnable(false);
	CollisionTracerTickFunction.TickGroup = TG_PostPhysics;
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
		if (SetupActorComponentTickFunction(&CollisionTracerTickFunction))
		{
			CollisionTracerTickFunction.AddPrerequisite(this, this->PrimaryComponentTick);
			CollisionTracerTickFunction.Target = this;
		}
	}
	else
	{
		if (CollisionTracerTickFunction.IsTickFunctionRegistered())
		{
			CollisionTracerTickFunction.UnRegisterTickFunction();
		}
	}
}

void UMontageGraphComponent::ExecuteMontageCollisionTraceTick(float MontageAlpha)
{
	FTransform            OwnerTransform = AnimInstance->GetSkelMeshComponent()->GetComponentTransform();
	
	UCollisionTracer* CollisionData  = LinkInfo.CollisionDataPtr;
	if (CollisionData && CollisionData->SamplePositions.Num())
	{
		const int SampleIndex_Start = CollisionData->GetCollisionAlphaIndex(MontageAlpha);
		const int SampleIndex_End   = SampleIndex_Start + 1;


		FCollisionQueryParams Params(FName(TEXT("MontageCollisionTrace")), false);
		Params.AddIgnoredActor(GetOwner());
		Params.AddIgnoredActors(TraceActorsToIgnore);
		Params.bReturnPhysicalMaterial = true;
		Params.bDebugQuery = true;


		const FVector Start       = OwnerTransform.TransformPosition(CollisionData->SamplePositions[SampleIndex_Start]);
		const FVector End         = OwnerTransform.TransformPosition(CollisionData->SamplePositions[SampleIndex_End]);
		const FQuat   Orientation = CollisionData->SampleOrientations[SampleIndex_Start];


		const FVector   Extent = CollisionData->CollisionExtent * .5f;
		FCollisionShape CollisionShape;
		CollisionShape = FCollisionShape::MakeBox(Extent);

		TArray<FHitResult> Hits;
		GetWorld()->SweepMultiByChannel(Hits, Start, End, Orientation, CollisionData->TraceChannel,
		                                CollisionShape,
		                                Params);

		TArray<UGameplayEffect*> EffectsToApply;
		for (int i : CollisionData->AnimSampleRange.TargetGameplayEffectIndices)
		{
			if (Graph->GameplayEffects.IsValidIndex(i))
			{
				EffectsToApply.Add(Graph->GameplayEffects[i].GetDefaultObject());
			}
		}


		if (Hits.Num() > 0)
		{
			OnCollisionHitActorsChanged.ExecuteIfBound(Hits, EffectsToApply);
		}


#if !UE_BUILD_SHIPPING

		if (Hits.Num() > 0)
		{
			DebugHitCollisionIndices.Add(SampleIndex_Start);
			DebugHitCollisionIndices.Add(SampleIndex_End);
		}
		
		if (MontageGraphCVars::bShowCollisionTracers)
		{
			for (int j = 0; j < CollisionData->SamplePositions.Num(); j++)
			{
				FColor DefaultColor = j < CollisionData->GetCollisionAlphaIndex(MontageAlpha) ?  FColor(40, 186, 30, 50) : FColor(119, 143, 125, 50);
				
				FColor HitColor =  FColor(255, 0, 15, 100);

				const FVector DebugStart = OwnerTransform.TransformPosition(CollisionData->SamplePositions[j]);
				const FQuat DebugOrientation = OwnerTransform.TransformRotation(CollisionData->SampleOrientations[j]);

				DrawDebugBox(GetWorld(), DebugStart, Extent, DebugOrientation,
				             DebugHitCollisionIndices.Contains(j) ? HitColor : DefaultColor, false, 0.0f, 1, 2.f);
			}
		}
#endif
	}
}


void UMontageGraphComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Fast Arrays don't use push model, but there's no harm in marking them with it.
	// The flag will just be ignored.
	FDoRepLifetimeParams Params;
	Params.Condition = COND_None;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UMontageGraphComponent, RepAnimMontageInfo, Params);
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UMontageGraphComponent::Init(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	AnimInstance = ASC->AbilityActorInfo->GetAnimInstance();

#if WITH_EDITOR
	FMontageGraphDelegates::OnGraphInitialized.Broadcast(this);
#endif
}

void UMontageGraphComponent::SetTraceActorsToIgnore(TArray<AActor*> ActorsToIgnore)
{
	TraceActorsToIgnore = ActorsToIgnore;
}


void UMontageGraphComponent::SelectLinkNode(const FGameplayTag& BindTag,
                                                  const FGameplayTagContainer& AdditionalTags,
                                                  FMGSelectionInfo& OutInfo)
{
	if (!Graph || !ASC || !Graph->RootNodes.Contains(BindTag))
	{
		OutInfo.bSelectionSuccessful = false;
		return;
	}

	FGameplayTagContainer TagsToMatch = ASC->GetOwnedGameplayTags();
	TagsToMatch.AppendTags(AdditionalTags);

		
	UMGNode* QueryNode = Graph->RootNodes[BindTag];
	UMGNode* NodeToTransitionTo = QueryNode;
	while (QueryNode)
	{
		// Find the first valid transition
		for (int i = 0; i < QueryNode->Edges.Num(); ++i)
		{
			const UMGEdge* NodeEdge = QueryNode->Edges[i];

			if (auto TagQueryEdge = Cast<UMGEdge_TagQuery>(NodeEdge))
			{
				if (TagQueryEdge->Query.Matches(TagsToMatch))
				{
					NodeToTransitionTo = QueryNode->ChildrenNodes[i];
					break;
				}
			}

			if (auto StackCountEdge = Cast<UMGEdge_StackCount>(NodeEdge))
			{
				int32 StackCount = ASC->GetAggregatedStackCount(StackCountEdge->Query);

				if (StackCount >= StackCountEdge->RequiredNumOfStacks)
				{
					NodeToTransitionTo = QueryNode->ChildrenNodes[i];
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

	if (SelectedNode)
	{
		OutInfo.FromNodeID = SelectedNode->ID;
	}

	SelectedNode   = NodeToTransitionTo;
	OutInfo.NodeID = SelectedNode->ID;


	OnNodeSelected.Broadcast(SelectedNode, 0);
#if WITH_EDITOR
	FMontageGraphDelegates::OnGraphNodeSelected.Broadcast(this, *SelectedNode);
#endif
	OutInfo.bSelectionSuccessful = true;
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

void UMontageGraphComponent::StopNodeMontageWithBlend(float OverrideBlendOutTime)
{
	StopLinkCollisionTick();
	
	UAnimMontage* MontageToStop = GetMontageForNodeID(RepAnimMontageInfo.NodeID);
	bool bShouldStopMontage = AnimInstance && MontageToStop && AnimInstance->Montage_IsPlaying(MontageToStop);
	
	if (bShouldStopMontage)
	{
		const float BlendOutTime = (OverrideBlendOutTime >= 0.0f ? OverrideBlendOutTime : MontageToStop->BlendOut.GetBlendTime());
		AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);
	}
}

void UMontageGraphComponent::PlayNodeMontage(uint16 NodeID)
{
	if (!ASC || !ASC->AbilityActorInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s]: Attempted to play MontageNode with no ASC"),
		       *GetFullNameSafe(this));
		return;
	}

	// if (!Graph->Montages.Contains(RepAnimMontageInfo.NodeID))
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("[%s]: Attempted to an unitialized MontageGraph Montage"),
	// 	       *GetFullNameSafe(this));
	// 	return;
	// }

	//
	// /*if we recieved the montage start time from the server on the predicting client,
	//  * we just advance to by the delta time Otherwise, we start the montage */
	// if (AnimInstance->Montage_IsActive(MontageToPlay))
	// {
	// 	AnimInstance->Montage_SetPosition(MontageToPlay, StartOffset);
	// }
	// else
	// {
	// }

	UCollisionTracer* CollisionDataPtr = Graph->CollisionTracers[NodeID];
	UAnimMontage*     MontageToPlay    = GetMontageForNodeID(NodeID);

	const float SweepStartTime = CollisionDataPtr->AnimSampleRange.AnimStartAlpha * MontageToPlay->GetPlayLength();
	const float SweepEndTime = CollisionDataPtr->AnimSampleRange.AnimEndAlpha * MontageToPlay->GetPlayLength();
	GetWorld()->GetTimerManager().SetTimer(StartTimeHandle, FTimerDelegate::CreateLambda([this]()
	{
		OnSweepStateChanged.Broadcast(true);
	}), SweepStartTime, false);


	GetWorld()->GetTimerManager().SetTimer(StopTimeHandle, FTimerDelegate::CreateLambda([this]()
	{
		OnSweepStateChanged.Broadcast(false);
	}), SweepEndTime , false);
	

	AnimInstance->Montage_Play(MontageToPlay, 1.f);
}

void UMontageGraphComponent::OnRep_ReplicatedAnimMontage()
{
	if (!AnimInstance)
	{
		return;
	}


	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		PlayNodeMontage(RepAnimMontageInfo.NodeID);
		//
		// UAnimMontage* MontageToPlay = GetMontageForNodeID(RepAnimMontageInfo.NodeID);
		//
		// if (AGameStateBase* GameState = GetWorld()->GetGameState())
		// {
		// 	//Adjust PlayRate to catch up to Authoratative EndTime
		// 	const float MontageLength = MontageToPlay->GetPlayLength();
		// 	const float LocalTime = GameState->GetServerWorldTimeSeconds();
		// 	const float SyncTimeDelta = (LocalTime + MontageLength) - RepAnimMontageInfo.SyncTime;
		// 	const float EffectivePlayRate = FMath::Abs(MontageLength + SyncTimeDelta) / MontageLength;
		//
		// 	if (AnimInstance->Montage_IsActive(MontageToPlay))
		// 	{
		// 		AnimInstance->Montage_SetPlayRate(MontageToPlay, EffectivePlayRate);
		// 	}
		// 	else
		// 	{
		// 		AnimInstance->Montage_Play(MontageToPlay, EffectivePlayRate);
		// 	}
		// }
	}
	else
	{
		//@TODO ~Tim: Adjust Predictive playback to match server
	}
}

void UMontageGraphComponent::OnRep_GraphID()
{
	// Graph = UMontageGraphGlobals::Get().LoadGraph(GraphID);
}

void UMontageGraphComponent::SetupLinkCollisionTick(const float MontageLength, const float LocalTime)
{
#if !UE_BUILD_SHIPPING
	DebugHitCollisionIndices.Reset();
#endif

	CollisionTracerTickFunction.StartTime = LocalTime;
	CollisionTracerTickFunction.EndTime   = LocalTime + MontageLength;
	CollisionTracerTickFunction.AccumulatedDilatedTime = 0.f;
	CollisionTracerTickFunction.SetTickFunctionEnable(true);
}

void UMontageGraphComponent::StopLinkCollisionTick()
{
	CollisionTracerTickFunction.SetTickFunctionEnable(false);
	GetWorld()->GetTimerManager().ClearTimer(StopTimeHandle);
	OnSweepStateChanged.Broadcast(false);
}

void UMontageGraphComponent::ServerLinkNode_Implementation(float PredictedSyncTime, uint16 LinkToNodeID,
                                                           FPredictionKey PredictionKey)
{
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		UAnimMontage* MontageToPlay = GetMontageForNodeID(LinkToNodeID);

		const float MontageLength = MontageToPlay->GetPlayLength();
		const float LocalTime = GameState->GetServerWorldTimeSeconds();
		const float SyncTimeDelta = (LocalTime + MontageToPlay->GetPlayLength()) - PredictedSyncTime;

		float EffectivePlayRate = FMath::Abs(MontageLength + SyncTimeDelta) / MontageLength;



		RepAnimMontageInfo.SyncTime = PredictedSyncTime;
		RepAnimMontageInfo.NodeID = LinkToNodeID;
		MARK_PROPERTY_DIRTY_FROM_NAME(UMontageGraphComponent, RepAnimMontageInfo, this);


		UCollisionTracer* CollisionDataPtr = Graph->CollisionTracers[LinkToNodeID];
		LinkInfo.CollisionHits.Reset(CollisionDataPtr->SamplePositions.Num());
		LinkInfo.CollisionDataPtr = CollisionDataPtr;
		SetupLinkCollisionTick(MontageLength, LocalTime);
		
		PlayNodeMontage(LinkToNodeID);
	}
}

void UMontageGraphComponent::LinkToNodeID_Predictive(uint16 NodeIDToLinkTo, FPredictionKey PredictionKey)
{
	if (!ASC || !ASC->AbilityActorInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s]: Attempted to link Node with no valid Ability System"),
		       *GetFullNameSafe(this));
		return;
	}

	UAnimMontage* MontageToPlay = GetMontageForNodeID(NodeIDToLinkTo);
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		float LocalPlayedAtTime = GameState->GetServerWorldTimeSeconds();
		ServerLinkNode(LocalPlayedAtTime + MontageToPlay->GetPlayLength(), NodeIDToLinkTo, PredictionKey);
	}

	LinkToNodeID(NodeIDToLinkTo);
}

void UMontageGraphComponent::LinkToNodeID(uint16 NodeID)
{
	if (!ASC || !ASC->AbilityActorInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s]: Attempted to link Node with no valid Ability System"),
		       *GetFullNameSafe(this));
		return;
	}

	UAnimMontage* MontageToPlay = GetMontageForNodeID(NodeID);
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Error,
		       TEXT("[%s]: Attempted to link Node with no valid Montage, have the montages been rebuilt?"),
		       *GetFullNameSafe(this));
		return;
	}
	const float MontageLength = MontageToPlay->GetPlayLength();

	if (GetOwnerRole() == ROLE_Authority)
	{
		if (AGameStateBase* GameState = GetWorld()->GetGameState())
		{
			const float LocalTime     = GameState->GetServerWorldTimeSeconds();


			RepAnimMontageInfo.NodeID   = NodeID;
			RepAnimMontageInfo.SyncTime = LocalTime + MontageLength;

			MARK_PROPERTY_DIRTY_FROM_NAME(UMontageGraphComponent, RepAnimMontageInfo, this);

			UCollisionTracer* CollisionDataPtr = Graph->CollisionTracers[NodeID];
			LinkInfo.CollisionHits.Reset(CollisionDataPtr->SamplePositions.Num());
			LinkInfo.CollisionDataPtr = CollisionDataPtr;
			SetupLinkCollisionTick(MontageLength, LocalTime);
		}
	}

	PlayNodeMontage(NodeID);
}

void UMontageGraphComponent::CancelNodeLink()
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;
	OnNodeLinkFailed.Broadcast();
}

UAnimMontage* UMontageGraphComponent::GetMontageForNodeID(uint16 NodeID)
{
	check(IsValid(Graph))
	UAnimMontage* MontageToPlay = Graph->Montages[NodeID];
	return MontageToPlay;
}
