// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "SMGNode.h"

class UMGEdNode_Selector;
class UMGEdNode;
class SHorizontalBox;

class SMGNode_Selector : public SMGNode
{
public:
	SLATE_BEGIN_ARGS(SMGNode_Selector){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMGEdNode_Selector* InNode);
	
	TSharedPtr<SHorizontalBox> OutNodeBox;
	virtual TSharedPtr<SToolTip> GetComplexTooltip() override;
	
	
protected:
	FSlateColor GetBorderBackgroundColor() const;
	virtual const FSlateBrush* GetNameIcon() const;
};
