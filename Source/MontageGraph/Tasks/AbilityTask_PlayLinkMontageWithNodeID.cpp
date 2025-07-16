// Copyright Drop Games Inc.


#include "AbilityTask_PlayLinkMontageWithNodeID.h"

#include "AbilitySystemComponent.h"
#include "MontageGraph/MontageGraphComponent.h"

UAbilityTask_PlayLinkMontageWithNodeID::UAbilityTask_PlayLinkMontageWithNodeID(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilityTask_PlayLinkMontageWithNodeID* UAbilityTask_PlayLinkMontageWithNodeID::LinkMontageNode(
	UGameplayAbility* OwningAbility, UMontageGraphComponent* InMontageGraphComponent, UMGNode* NodeToLink)
{
	UAbilityTask_PlayLinkMontageWithNodeID* MyObj = NewAbilityTask<UAbilityTask_PlayLinkMontageWithNodeID>(OwningAbility);
	MyObj->MontageGraphComponent = InMontageGraphComponent;
	MyObj->NodeIDToLinkTo = NodeToLink->ID;
	return MyObj;
}

UAbilityTask_PlayLinkMontageWithNodeID* UAbilityTask_PlayLinkMontageWithNodeID::PlayLinkedMontageWithNodeID(
	UGameplayAbility* OwningAbility, UMontageGraphComponent* InMontageGraphComponent, int NodeID)
{
	UAbilityTask_PlayLinkMontageWithNodeID* MyObj = NewAbilityTask<UAbilityTask_PlayLinkMontageWithNodeID>(OwningAbility);
	MyObj->MontageGraphComponent = InMontageGraphComponent;
	MyObj->NodeIDToLinkTo = NodeID;
	return MyObj;
}

void UAbilityTask_PlayLinkMontageWithNodeID::OnPredictiveLinkRejected()
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

void UAbilityTask_PlayLinkMontageWithNodeID::Activate()
{
	Super::Activate();

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get(), IsPredictingClient());

	if (IsValid(MontageGraphComponent))
	{
		MontageGraphComponent->OnCollisionHitActorsChanged.BindUObject(this, &ThisClass::OnCollisionTracerHitActorsChanged);
		MontageGraphComponent->OnNodeLinkFailed.AddUObject(this, &ThisClass::OnLinkFailed);

		if (IsPredictingClient())
		{
			UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
			if (!Ability || !ASC)
			{
				return;
			}
			FPredictionKey PredictionKey = ASC->GetPredictionKeyForNewAction();
			if (PredictionKey.IsValidKey())
			{
				PredictionKey.NewRejectedDelegate().BindUObject(
					this, &UAbilityTask_PlayLinkMontageWithNodeID::OnPredictiveLinkRejected);
			}

			MontageGraphComponent->LinkToNodeID_Predictive(NodeIDToLinkTo, PredictionKey);
		}
		else if (IsLocallyControlled())
		{
			MontageGraphComponent->LinkToNodeID(NodeIDToLinkTo);
		}
	}
}

void UAbilityTask_PlayLinkMontageWithNodeID::OnCollisionTracerHitActorsChanged(TArray<FHitResult>& Hits, TArray<UGameplayEffect*>& TargetEffects)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCollisionTracerHit.Broadcast(Hits, TargetEffects);
	}
}

void UAbilityTask_PlayLinkMontageWithNodeID::OnLinkFailed()
{
	EndTask();
}
