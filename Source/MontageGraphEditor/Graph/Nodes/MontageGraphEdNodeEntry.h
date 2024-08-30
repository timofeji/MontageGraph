// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphEdNode.h"
#include "MontageGraphEdNodeEntry.generated.h"

class UMontageGraphNode;
/** Editor Graph Node for entry point in Montage Graphs. Based off UAnimStateEntryNode for state machine graphs in Anim BP. */
UCLASS(MinimalAPI)
class UMontageGraphEdNodeEntry : public UMontageGraphEdNode
{
	GENERATED_BODY()

public:
	UMontageGraphEdNodeEntry();


	//~ Begin UEdGraphNode Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;

	virtual bool CanDuplicateNode() const override	{ return false; }
	//~ End UEdGraphNode Interface

	MONTAGEGRAPHEDITOR_API UEdGraphNode* GetOutputNode();
};
