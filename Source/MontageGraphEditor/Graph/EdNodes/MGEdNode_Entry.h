// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MGEdNode.h"
#include "MGEdNode_Entry.generated.h"

class UMGNode;
/** Editor Graph Node for entry point in Montage Graphs. Based off UAnimStateEntryNode for state machine graphs in Anim BP. */
UCLASS(MinimalAPI)
class UMGEdNode_Entry : public UMGEdNode
{
	GENERATED_BODY()

public:
	UMGEdNode_Entry();


	//~ Begin UEdGraphNode Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanDuplicateNode() const override	{ return false; }
	FLinearColor GetNodeTitleColor() const override;
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface

	MONTAGEGRAPHEDITOR_API UEdGraphNode* GetOutputNode();
};
