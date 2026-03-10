#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "MG_AnimNotifyState_LooseTag.generated.h"

/**
 * AnimNotifyState baked into a montage by UMontageTrack_GameplayState.
 * Adds loose gameplay tags to the owning actor's ASC for the duration of the section,
 * then removes them when the state ends.
 */
UCLASS(DisplayName = "MG Loose Tags")
class MONTAGEGRAPH_API UMG_AnimNotifyState_LooseTag : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayTagContainer Tags;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                         float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                       const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
