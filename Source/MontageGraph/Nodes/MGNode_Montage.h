//Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MGNode.h"
#include "MGNode_Montage.generated.h"


class UGameplayEffect;
class UCollisionTracer;

/**
 *  Base Class for MG Animation nodes acting based on an Anim Montage or Sequence asset.
 *
 *  Holds runtime properties for animation and effects / cues containers.
 */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMGNode_Montage : public UMGNode
{
	GENERATED_BODY()

public:
	UMGNode_Montage();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
	FGameplayTagContainer AggregatedTags;
	
	//
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	// TArray<TSubclassOf<UGameplayEffect>> TargetEffects;
	//
	// UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Gameplay")
	// UAnimMontage* Montage;
	//
	// UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Gameplay")
	// UCollisionTracerData* CollisionTracer;
	

	virtual bool SupportsAssetClass(UClass* AssetClass);
	virtual FText GetNodeTitle() const override;
#if WITH_EDITOR
	virtual FText GetAnimAssetLabel() const;
	virtual FText GetAnimAssetLabelTooltip() const;
	virtual FLinearColor GetBackgroundColor() const override;
#endif
};
