#include "MG_AnimNotify_GameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UMG_AnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!EventTag.IsValid() || !MeshComp) { return; }

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) { return; }

	FGameplayEventData Data = EventData;
	Data.EventTag           = EventTag;
	Data.Instigator         = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
}

FString UMG_AnimNotify_GameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid()
		? FString::Printf(TEXT("Event: %s"), *EventTag.GetTagName().ToString())
		: TEXT("MG Gameplay Event");
}
