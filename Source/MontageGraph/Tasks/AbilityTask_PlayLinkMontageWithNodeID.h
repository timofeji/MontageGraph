// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_PlayLinkMontageWithNodeID.generated.h"

class UMontageGraphComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionTracerHit, const TArray<FHitResult>&, HitResults,
                                             const TArray<UGameplayEffect*>&, EffectsToApply);
/**
 * 
 */
UCLASS()
class MONTAGEGRAPH_API UAbilityTask_PlayLinkMontageWithNodeID : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnCollisionTracerHit OnCollisionTracerHit;

	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayLinkMontageWithNodeID* LinkMontageNode(UGameplayAbility* OwningAbility,
	                                                     UMontageGraphComponent* InMontageGraphComponent,
	                                                     class UMGNode* NodeToLink);
	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayLinkMontageWithNodeID* PlayLinkedMontageWithNodeID(UGameplayAbility* OwningAbility,
	                                                     UMontageGraphComponent* InMontageGraphComponent,
	                                                     int NodeID);


	void OnPredictiveLinkRejected();

	UFUNCTION()
	void OnLinkFailed();

	virtual void Activate() override;

	private:

	uint16 NodeIDToLinkTo;
	UMontageGraphComponent* MontageGraphComponent;


protected:
	void OnCollisionTracerHitActorsChanged(TArray<FHitResult>& Hits, TArray<UGameplayEffect*>& TargetEffects);
};
