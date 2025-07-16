// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MontageGraph.h"
#include "Components/ActorComponent.h"
#include "Tracers/MontageCollisionTracer.h"
#include "MontageGraphComponent.generated.h"

class UMGNode_Montage;


/** Data about montages that is replicated to simulated clients */
USTRUCT()
struct MONTAGEGRAPH_API FMontageGraphRepInfo
{
	GENERATED_USTRUCT_BODY()

	/** ID of the MontageNode we're playing*/
	UPROPERTY()
	uint16 NodeID;
	
	UPROPERTY()
	float SyncTime;
	
	UPROPERTY(NotReplicated)
	float PredictionTime;
	
	/** Play Rate */
	UPROPERTY()
	float PlayRate;

	/** Montage position */
	UPROPERTY(NotReplicated)
	float Position;
	
	UPROPERTY()
	FPredictionKey PredictionKey;

	/** The current section Id used by the montage. Will only be valid if bRepPosition is false */
	UPROPERTY()
	uint8 SectionIdToPlay;

	FMontageGraphRepInfo()
	: NodeID(0),
	PlayRate(0.f),
	Position(0.f),
	SectionIdToPlay(0),
	SyncTime(0.f),
	PredictionTime(0.f)
	{
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FMontageGraphRepInfo> : public TStructOpsTypeTraitsBase2<FMontageGraphRepInfo>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct MONTAGEGRAPH_API FMGSelectionInfo
{
	
	GENERATED_USTRUCT_BODY()

	/** ID of the MontageNode we just selected*/
	UPROPERTY()
	uint16 NodeID = 0;

	/** ID of the MontageNode  that was last selected*/
	UPROPERTY()
	uint16 FromNodeID = 0;

	UPROPERTY()
	bool bSelectionSuccessful;
};


USTRUCT(BlueprintType)
struct MONTAGEGRAPH_API FMontageLinkInfo
{
	
	GENERATED_USTRUCT_BODY()

	/** ID of the MontageNode that was last linked*/
	UPROPERTY()
	uint16 FromNodeID= 0;
	
	UPROPERTY()
	uint16 ToNodeID = 0;
	
	UPROPERTY()
	UCollisionTracer* CollisionDataPtr;
	
	UPROPERTY()
	TArray<FHitResult> CollisionHits;

};

class UAbilitySystemComponent;


DECLARE_MULTICAST_DELEGATE_TwoParams(FMontageNodeDelegate, UMGNode*, uint32)
DECLARE_MULTICAST_DELEGATE_OneParam(FBindDelegate, FGameplayTag)
DECLARE_MULTICAST_DELEGATE(FMontageNodeLinkDelegate)


DECLARE_DELEGATE_TwoParams(FCollisionHitActorsChanged, TArray<FHitResult>&, TArray<UGameplayEffect*>&)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageNodeCollisionEvent, bool, bIsStart);

class UMontageGraph;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MONTAGEGRAPH_API UMontageGraphComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMontageGraphComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MontageGraph")
	FTransform CollisionTracerOffset;

	UFUNCTION(BlueprintCallable)
	void Init(UAbilitySystemComponent* ASC);
	
	
	UFUNCTION(BlueprintCallable)
	void SetTraceActorsToIgnore(TArray<AActor*> ActorsToIgnore);
	
	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	virtual void ExecuteMontageCollisionTraceTick(float MontageAlpha);



	//Filters the branch we should be comboing, based on Selector nodes, then selects appropriate montage based on the ComboIndex
	void SelectLinkNode(const FGameplayTag& BindTag,
	                          const FGameplayTagContainer& AdditionalTags,
	                          FMGSelectionInfo& OutInfo);
	


	void SetGraph(UMontageGraph* MontageGraph);
	const UMontageGraph* GetGraph() const;
	
	void StopNodeMontageWithBlend(float OverrideBlendOutTime);
	void PlayNodeMontage(uint16 NodeID);


	FBindDelegate OnBindReset(const FGameplayTag& BindTag, const FGameplayTagContainer& AdditionalTags);



	UFUNCTION(Server, Reliable)
	void ServerLinkNode(float PredictedSyncTime, uint16 LinkToNodeID,  FPredictionKey PredictionKey);
	
	void LinkToNodeID(uint16 NodeID);
	void LinkToNodeID_Predictive(uint16 NodeID, FPredictionKey PredictionKey);
	
	void CancelNodeLink();

	UAnimMontage* GetMontageForNodeID(uint16 NodeID);


	UMGNode* SelectedNode;
	FMontageNodeDelegate OnNodeSelected;
	FMontageNodeDelegate OnNodeCommitted;

	UFUNCTION()
	virtual void OnRep_ReplicatedAnimMontage();
	
	UFUNCTION()
	virtual void OnRep_GraphID();
	void         SetupLinkCollisionTick(float MontageLength, float LocalTime);
	void         StopLinkCollisionTick();

	FMontageNodeCollisionEvent OnSweepStateChanged;
	
	FMontageNodeLinkDelegate OnNodeLinkFailed;
	
	FCollisionHitActorsChanged OnCollisionHitActorsChanged;
protected:

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FMontageGraphRepInfo RepAnimMontageInfo;
	
	UPROPERTY(Transient)
	FMontageLinkInfo LinkInfo;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_GraphID)
	uint16 GraphID;

	FMontageCollisionTracerTickFunction  CollisionTracerTickFunction;
private:
	UMontageGraph* Graph;
	UAbilitySystemComponent* ASC;
	UAnimInstance* AnimInstance;

	TArray<AActor*> TraceActorsToIgnore;
	
	
	FTimerHandle StartTimeHandle;
	FTimerHandle StopTimeHandle;
	
#if !UE_BUILD_SHIPPING
	TArray<int> DebugHitCollisionIndices;
#endif
};


