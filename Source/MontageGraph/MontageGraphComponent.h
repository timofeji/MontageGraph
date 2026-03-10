// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "GameplayEffectTypes.h"
#include "MontageGraph.h"
#include "Components/ActorComponent.h"
#include "Tracers/MontageCollisionTracer.h"
#include "MontageGraphComponent.generated.h"

class UMGNode_Montage;

UENUM(BlueprintType)
enum class EMontageGraphState : uint8
{
	Idle,
	NodeSelected,
	NodeQueued,
	LinkActive,
	BlendingOut
};


/** Data about montages that is replicated to simulated clients */
USTRUCT()
struct MONTAGEGRAPH_API FMGReplicatedLinkInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	uint16 NodeID = 0;
	
	UPROPERTY()
	float SyncTime = 0.f;
	
	/** Play Rate */
	UPROPERTY()
	float PlayRate = 1.f;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FMGReplicatedLinkInfo> : public TStructOpsTypeTraitsBase2<FMGReplicatedLinkInfo>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct MONTAGEGRAPH_API FMontageLink
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGameplayEffectContextHandle EffectContextHandle;

	/** Bit array for validated collision frames - more memory efficient than TArray<bool> */
	TBitArray<> ValidatedFrameIndices;

	UPROPERTY()
	UCollisionTracer* TracerPtr = nullptr;

	UPROPERTY()
	UMGNode* LinkedNode = nullptr;

	UPROPERTY()
	UAnimMontage* Montage = nullptr;

	UPROPERTY()
	int32 LastValidatedFrame = 0;

	UPROPERTY()
	float StartTime = 0.f;

	UPROPERTY()
	float TargetLinkToTime = 0.f;

	/** Optional blend settings - avoids dangling pointer issues */
	TOptional<FMontageBlendSettings> BlendSettings;

	void InitializeCollision(UCollisionTracer* CollisionTracer)
	{
		TracerPtr = CollisionTracer;
		const int32 NumSamples = TracerPtr ? TracerPtr->SamplePositions.Num() : 0;
		ValidatedFrameIndices.Init(false, NumSamples);
		LastValidatedFrame = 0;
	}

	void Reset()
	{
		LinkedNode = nullptr;
		TracerPtr = nullptr;
		Montage = nullptr;
		BlendSettings.Reset();
		LastValidatedFrame = 0;
		ValidatedFrameIndices.Empty();
	}

	FORCEINLINE bool IsFrameValidated(int32 FrameIndex) const
	{
		return ValidatedFrameIndices.IsValidIndex(FrameIndex) && ValidatedFrameIndices[FrameIndex];
	}

	FORCEINLINE void SetFrameValidated(int32 FrameIndex, bool bValidated)
	{
		if (ValidatedFrameIndices.IsValidIndex(FrameIndex))
		{
			ValidatedFrameIndices[FrameIndex] = bValidated;
		}
	}
};

USTRUCT()
struct MONTAGEGRAPH_API FCollisionSweepIntersection
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(Transient)
	const AActor* Actor = nullptr;
	
	uint8 NumHits = 0;
	
};


DECLARE_DELEGATE_TwoParams(FCollisionHitActorsChanged, TArray<FHitResult>&, TArray<UGameplayEffect*>&)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageNodeCollisionEvent, bool, bIsStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageNodeDelegate, UMGNode*, MontageNode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLinkedMontageDelegate);


class UAbilitySystemComponent;
class UMontageGraph;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MONTAGEGRAPH_API UMontageGraphComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMontageGraphComponent();
	

	UFUNCTION(BlueprintCallable)
	void Init(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable)
	void SetTraceActorsToIgnore(TArray<AActor*> ActorsToIgnore);
	
	UFUNCTION(BlueprintCallable)
	virtual void SelectNode(UMGNode* NodeToTransitionTo);

	UFUNCTION(BlueprintCallable)
	void SetGraph(UMontageGraph* MontageGraph);
	
	UFUNCTION(BlueprintCallable)
	const UMontageGraph* GetGraph() const;

	UFUNCTION(BlueprintCallable)
	EMontageGraphState GetCurrentState() const { return CurrentState; }

	virtual void ResetActiveLink();

	UFUNCTION(Server, Reliable)
	void ServerQueueNodeLink(uint16 NodeID);

	UFUNCTION()
	void QueueNodeLink(UMGNode* NodeToLink, bool bNotifyServer = false);
	
	void LinkQueuedNode();
	
	UFUNCTION(Server, Reliable)
	void ServerLinkNode(uint16 NodeIDToLink, float PredictionStartTime, FPredictionKey PredictionKey);
	
	UFUNCTION(Client, Reliable)
	void ClientPredictedLinkRejected(uint16 RejectedNodeID);
	
	UFUNCTION(Client, Reliable)
	void ClientPredictedLinkAccepted(uint16 AcceptedNodeID);

	void ActivateMontageLink(UMGNode* NodeToLink, UAnimMontage* MontageToPlay);
	
	UFUNCTION()
	void LinkMontageNode(UMGNode* NodeToLink, const bool bIsPredicting = false);
	
	UFUNCTION()
	void CancelNodeLink();
	
	UFUNCTION()
	void OnPredictiveLinkRejected(uint16 NodeID);
	
	//Filters the branch we should be comboing, based on edge transition rules nodes, then selects appropriate montage based on the ComboIndex
	UMGNode* FindNode(const FGameplayTag& BindTag, const FGameplayTagContainer& AdditionalTags = FGameplayTagContainer());

	virtual bool CanEnqueueNode() const;

	UAnimMontage* GetMontageForNodeID(uint16 NodeID) const;
	UAnimMontage* GetActiveLinkMontage() const { return ActiveLink.Montage; }
	
	void StartLinkedMontageSweep(const UAnimMontage* Montage);
	void StopLinkedMontageTick();
	void StopLinkedMontage(float BlendTimeOverride);

	FMontageNodeCollisionEvent OnSweepStateChanged;
	FCollisionHitActorsChanged OnCollisionHitActorsChanged;

	UPROPERTY(EditAnywhere, BlueprintAssignable)
	FMontageNodeDelegate OnNodeSelected;

	UPROPERTY(EditAnywhere, BlueprintAssignable)
	FMontageNodeDelegate OnNodeLinked;

	UPROPERTY(EditAnywhere, BlueprintAssignable)
	FMontageNodeDelegate OnNodeLinkFailed;

	UPROPERTY()
	UMGNode* QueuedNode = nullptr;

	UPROPERTY()
	UMGNode* SelectedNode = nullptr;

	// TArray<uint16> QueuedNodes;

protected:

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FMGReplicatedLinkInfo RepLinkInfo;
	
	UFUNCTION()
	virtual void OnRep_ReplicatedAnimMontage();
	
	UPROPERTY(Transient)
	FMontageLink ActiveLink;
	
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMontageGraph> Graph;
	
	UPROPERTY(BlueprintReadOnly)
	UAbilitySystemComponent* ASC;

	/** Cached AnimInstance pointer - refreshed when ASC changes */
	UPROPERTY(Transient)
	TWeakObjectPtr<UAnimInstance> CachedAnimInstance;

	/** Current state of the montage graph */
	UPROPERTY(BlueprintReadOnly, Transient)
	EMontageGraphState CurrentState = EMontageGraphState::Idle;

	UAnimInstance* GetAnimInstance() const;


	/** Sets the current state and handles state transition logic */
	void SetState(EMontageGraphState NewState);

	virtual void BeginPlay() override;
	
	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	//Collision Samples get transformed by this, overload to add crouching support, etc
	virtual FTransform GetTransformForCollisionTracer(UCollisionTracer* CollisionData);
	
	virtual void ExecuteMontageCollisionTraceTick(float AnimAlpha);

	virtual void NotifyIntersectionUpdated(const FCollisionSweepIntersection& Intersection, const FHitResult& Hit){ }

	virtual void NotifyBeginIntersection(const FCollisionSweepIntersection& Intersection, const FHitResult& Hit){ }

	virtual void NotifyNodeLinked(const UMGNode* LinkedNode) {};
	virtual void NotifyLinkBlendingOut(UMGNode* LinkedNode);
	
	virtual void NotifyNodeSelected();
	virtual void NotifyCollisionSweepStarted();
	virtual void NotifyCollisionSweepEnded();
	

protected:


	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* AnimMontage, bool bInterrupted);
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* AnimMontage, bool bInterrupted);
	FOnMontageEnded MontageEndedDelegate;
	

	/* Actors that will be ignored for collision detection */
	TArray<AActor*> TraceActorsToIgnore;

	/** Actors we're currently intersecting in the sweep - uses TMap for O(1) lookup instead of O(n) */
	TMap<const AActor*, FCollisionSweepIntersection> ActorIntersectionMap;

	/** Reusable array for frames to validate - avoids per-tick allocation */
	TArray<int32> FramesToValidate;

	/** Pooled hit results array to avoid per-sweep allocation */
	TArray<FHitResult> PooledHitResults;

	FTimerHandle StartCollisionTimerHandle;
	FTimerHandle StopCollisionTimerHandle;
	FTimerHandle QueuedLinkTimerHandle;
	
	
	FTimerDelegate StartCollisionSweepDelegate;
	FTimerDelegate StopCollisionSweepDelegate;
	FTimerDelegate QueuedLinkDelegate;
	
	FMontageCollisionTracerTickFunction  SweepTickFunc;



	friend struct FMontageCollisionTracerTickFunction;

	//*Debug*//
public:
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
protected:
	
	bool IsPredicting() const;
	
	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos);
	
	#if !UE_BUILD_SHIPPING
	TArray<int32> DebugHitCollisionIndices;
	#endif
	
	
};


