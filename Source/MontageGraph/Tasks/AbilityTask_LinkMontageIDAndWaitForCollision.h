// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_LinkMontageIDAndWaitForCollision.generated.h"

class UMontageGraphComponent;

UCLASS()
class MONTAGEGRAPH_API UAbilityTask_LinkMontageIDAndWaitForCollision : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
public:
	
	// UPROPERTY(BlueprintAssignable)
	// FOnCollisionTracerHit OnCollisionTracerHit;
	//
	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_LinkMontageIDAndWaitForCollision* LinkMontageNodeAndWaitForCollision(UGameplayAbility* OwningAbility,
	                                                     UMontageGraphComponent* InMontageGraphComponent,
	                                                     class UMGNode* NodeToLink);
	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_LinkMontageIDAndWaitForCollision* LinkMontageNodeIDAndWaitForCollision(UGameplayAbility* OwningAbility,
	                                                     UMontageGraphComponent* InMontageGraphComponent,
	                                                     int NodeID);


	void OnPredictiveLinkRejected();

	UFUNCTION()
	void OnLinkFailed();

	virtual void Activate() override;

	private:

	uint16 NodeIDToLinkTo;
	UMontageGraphComponent* MontageGraphComponent;
FPredictionKey PredictionKey;

protected:
	void OnCollisionTracerHitActorsChanged(TArray<FHitResult>& Hits, TArray<UGameplayEffect*>& TargetEffects);
};
