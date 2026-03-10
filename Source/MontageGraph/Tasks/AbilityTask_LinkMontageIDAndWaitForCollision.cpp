// Copyright Drop Games Inc.


#include "AbilityTask_LinkMontageIDAndWaitForCollision.h"

#include "AbilitySystemComponent.h"
#include "MontageGraph/MontageGraphComponent.h"

UAbilityTask_LinkMontageIDAndWaitForCollision::UAbilityTask_LinkMontageIDAndWaitForCollision(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilityTask_LinkMontageIDAndWaitForCollision* UAbilityTask_LinkMontageIDAndWaitForCollision::LinkMontageNodeAndWaitForCollision(
	UGameplayAbility* OwningAbility, UMontageGraphComponent* InMontageGraphComponent, UMGNode* NodeToLink)
{
	UAbilityTask_LinkMontageIDAndWaitForCollision* MyObj = NewAbilityTask<UAbilityTask_LinkMontageIDAndWaitForCollision>(OwningAbility);
	MyObj->MontageGraphComponent = InMontageGraphComponent;
	MyObj->NodeIDToLinkTo = NodeToLink->ID;
	return MyObj;
}

UAbilityTask_LinkMontageIDAndWaitForCollision* UAbilityTask_LinkMontageIDAndWaitForCollision::LinkMontageNodeIDAndWaitForCollision(
	UGameplayAbility* OwningAbility, UMontageGraphComponent* InMontageGraphComponent, int NodeID)
{
	UAbilityTask_LinkMontageIDAndWaitForCollision* MyObj = NewAbilityTask<UAbilityTask_LinkMontageIDAndWaitForCollision>(OwningAbility);
	MyObj->MontageGraphComponent = InMontageGraphComponent;
	MyObj->NodeIDToLinkTo = NodeID;
	return MyObj;
}

void UAbilityTask_LinkMontageIDAndWaitForCollision::OnPredictiveLinkRejected()
{

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, *FString::Printf(TEXT("[%s]: ATTACK REJECTED BY SERVER"),
	                                                                        *UEnum::GetValueAsString(
		                                                                        TEXT("Engine.ENetRole"),
		                                                                        GetOwnerActor()->GetLocalRole())));

	if (MontageGraphComponent)
	{
		MontageGraphComponent->CancelNodeLink();
	}
}

void UAbilityTask_LinkMontageIDAndWaitForCollision::Activate()
{
	Super::Activate();
	//
	// FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(), IsPredictingClient());
	//
	// if (IsValid(MontageGraphComponent) && MontageGraphComponent->CanEnqueueNode(NodeIDToLinkTo))
	// {
	// 	MontageGraphComponent->OnCollisionHitActorsChanged.BindUObject(this, &ThisClass::OnCollisionTracerHitActorsChanged);
	// 	// MontageGraphComponent->OnNodeLinkFailed.AddUObject(this, &ThisClass::OnLinkFailed);
	//
	// 	if (IsPredictingClient())
	// 	{
	// 		UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	// 		if (!Ability || !ASC)
	// 		{
	// 			return;
	// 		}
	// 		PredictionKey = ASC->GetPredictionKeyForNewAction();
	// 		if (PredictionKey.IsValidKey())
	// 		{
	// 			PredictionKey.NewRejectedDelegate().BindUObject(
	// 				this, &UAbilityTask_LinkMontageIDAndWaitForCollision::OnPredictiveLinkRejected);
	// 		}
	//
	// 		MontageGraphComponent->LinkToNodeID_Predictive(NodeIDToLinkTo, PredictionKey);
	// 	}
	// 	else if (IsLocallyControlled())
	// 	{
	// 		MontageGraphComponent->LinkNodeID(NodeIDToLinkTo);
	// 	}
	// }
}

void UAbilityTask_LinkMontageIDAndWaitForCollision::OnCollisionTracerHitActorsChanged(TArray<FHitResult>& Hits, TArray<UGameplayEffect*>& TargetEffects)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// OnCollisionTracerHit.Broadcast(Hits, TargetEffects);
	}
}

void UAbilityTask_LinkMontageIDAndWaitForCollision::OnLinkFailed()
{
	EndTask();
}
