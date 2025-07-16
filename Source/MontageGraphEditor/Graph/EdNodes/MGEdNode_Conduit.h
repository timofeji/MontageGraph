// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MGEdNode.h"
#include "MGEdNode_Conduit.generated.h"



/** Editor Graph Node for Conduit nodes in Montage Graphs. Based off UAnimStateConduitNode for state machine graphs in Anim BP. */
UCLASS(MinimalAPI)
class UMGEdNode_Conduit : public UMGEdNode
{
	GENERATED_BODY()

public:
	UMGEdNode_Conduit();
	


	//~ Begin UEdGraphNode Interface
	virtual FText   GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText   GetTooltipText() const override;
	virtual bool    CanDuplicateNode() const override { return false; }
	virtual void    ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	FLinearColor GetNodeTitleColor() const override;
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface
};
