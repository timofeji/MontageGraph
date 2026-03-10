// Copyright Drop Games Inc.


#include "AbilityTask_LinkMontageByID.h"

#include "AbilitySystemComponent.h"
#include "MontageGraph/MontageGraphComponent.h"

UAbilityTask_LinkMontageByID::UAbilityTask_LinkMontageByID(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilityTask_LinkMontageByID* UAbilityTask_LinkMontageByID::LinkMontageNode(
	UGameplayAbility* OwningAbility, UMontageGraphComponent* InMontageGraphComponent, UMGNode* NodeToLink)
{
	UAbilityTask_LinkMontageByID* MyObj = NewAbilityTask<UAbilityTask_LinkMontageByID>(OwningAbility);
	MyObj->MontageGraphComponent = InMontageGraphComponent;
	MyObj->NodeIDToLinkTo = NodeToLink->ID;
	return MyObj;
}

UAbilityTask_LinkMontageByID* UAbilityTask_LinkMontageByID::LinkMontage(
	UGameplayAbility* OwningAbility, UMontageGraphComponent* InMontageGraphComponent, int NodeID)
{
	UAbilityTask_LinkMontageByID* MyObj = NewAbilityTask<UAbilityTask_LinkMontageByID>(OwningAbility);
	MyObj->MontageGraphComponent = InMontageGraphComponent;
	MyObj->NodeIDToLinkTo = NodeID;
	return MyObj;
}

void UAbilityTask_LinkMontageByID::OnPredictiveLinkRejected()
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

void UAbilityTask_LinkMontageByID::Activate()
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
	// 				this, &UAbilityTask_LinkMontageByID::OnPredictiveLinkRejected);
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

void UAbilityTask_LinkMontageByID::OnCollisionTracerHitActorsChanged(TArray<FHitResult>& Hits, TArray<UGameplayEffect*>& TargetEffects)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCollisionTracerHit.Broadcast(Hits, TargetEffects);
	}
}

void UAbilityTask_LinkMontageByID::OnLinkFailed()
{
	EndTask();
}
