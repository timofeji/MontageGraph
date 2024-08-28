//Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphNode.h"
#include "MontageGraphNode_Animation.generated.h"


/**
 *  Base Class for MG Animation nodes acting based on an Anim Montage or Sequence asset.
 *
 *  Holds runtime properties for animation and effects / cues containers.
 */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMontageGraphNode_Animation : public UMontageGraphNode
{
	GENERATED_BODY()

public:
	UMontageGraphNode_Animation();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta=(DisplayPriority=1))
	UAnimMontage* AnimationMontage;

	virtual bool SupportsAssetClass(UClass* AssetClass);
	virtual FText GetNodeTitle() const override;

#if WITH_EDITOR
	virtual FText GetAnimAssetLabel() const;
	virtual FText GetAnimAssetLabelTooltip() const;
	virtual FLinearColor GetBackgroundColor() const override;
#endif
};
