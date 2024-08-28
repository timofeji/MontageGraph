//Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphNode.h"
#include "MontageGraphNode_Action.generated.h"


/**
 *  Base Class for Combo Graph nodes acting based on an Anim Sequence asset.
 *
 *  Holds runtime properties for animation and effects / cues containers.
 */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMontageGraphNode_Action : public UMontageGraphNode
{
	GENERATED_BODY()

public:
	UMontageGraphNode_Action();

	virtual bool SupportsAssetClass(UClass* AssetClass);
	virtual FText GetNodeTitle() const override;

#if WITH_EDITOR
	virtual FText GetAnimAssetLabel() const;
	virtual FText GetAnimAssetLabelTooltip() const;
	virtual FLinearColor GetBackgroundColor() const override;
#endif
};
