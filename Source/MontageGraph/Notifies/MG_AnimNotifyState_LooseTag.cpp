#include "MG_AnimNotifyState_LooseTag.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"

void UMG_AnimNotifyState_LooseTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                               float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (Tags.IsEmpty() || !MeshComp) { return; }
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) { return; }

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
	{
		ASC->AddLooseGameplayTags(Tags);
	}
}

void UMG_AnimNotifyState_LooseTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (Tags.IsEmpty() || !MeshComp) { return; }
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) { return; }

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
	{
		ASC->RemoveLooseGameplayTags(Tags);
	}
}

FString UMG_AnimNotifyState_LooseTag::GetNotifyName_Implementation() const
{
	if (Tags.IsEmpty()) { return TEXT("MG Loose Tags"); }
	return FString::Printf(TEXT("Tags: %s"), *Tags.ToStringSimple(false));
}
