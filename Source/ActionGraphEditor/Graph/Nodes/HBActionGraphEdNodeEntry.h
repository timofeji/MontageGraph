// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "ActionGraphEdNode.h"
#include "HBActionGraphEdNodeEntry.generated.h"

class UActionGraphNode;
/** Editor Graph Node for entry point in Action Graphs. Based off UAnimStateEntryNode for state machine graphs in Anim BP. */
UCLASS(MinimalAPI)
class UActionGraphEdNodeEntry : public UActionGraphEdNode
{
	GENERATED_BODY()

public:
	UActionGraphEdNodeEntry();


	//~ Begin UEdGraphNode Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;

	virtual bool CanDuplicateNode() const override	{ return false; }
	//~ End UEdGraphNode Interface

	ACTIONGRAPHEDITOR_API UEdGraphNode* GetOutputNode();
};
