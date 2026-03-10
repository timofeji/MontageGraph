#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MG_AnimNotify_GameplayEvent.generated.h"

/**
 * AnimNotify baked into a montage by UMontageTrack_GameplayEvent.
 * At notify time, sends a gameplay event to the owning actor's ASC.
 */
UCLASS(DisplayName = "MG Gameplay Event")
class MONTAGEGRAPH_API UMG_AnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayEventData EventData;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
