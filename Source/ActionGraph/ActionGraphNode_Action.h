//Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "ActionGraphNode.h"
#include "ActionGraphNode_Action.generated.h"


/**
 *  Base Class for Combo Graph nodes acting based on an Anim Sequence asset.
 *
 *  Holds runtime properties for animation and effects / cues containers.
 */
UCLASS(Blueprintable)
class ACTIONGRAPH_API UActionGraphNode_Action : public UActionGraphNode
{
	GENERATED_BODY()

public:
	UActionGraphNode_Action();

	virtual bool SupportsAssetClass(UClass* AssetClass);
	virtual FText GetNodeTitle() const override;

#if WITH_EDITOR
	virtual FText GetAnimAssetLabel() const;
	virtual FText GetAnimAssetLabelTooltip() const;
	virtual FLinearColor GetBackgroundColor() const override;
#endif
};
