// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "SGraphNodeAction.h"

class UMontageGraphEdNodeSelector;
class UMontageGraphEdNode;
class SHorizontalBox;

class SMontageGraphNodeSelector : public SGraphNodeAction
{
public:
	SLATE_BEGIN_ARGS(SMontageGraphNodeSelector){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMontageGraphEdNodeSelector* InNode);
	void RefreshOutputPins();

	// SNodePanel::SNode interface
	// virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
	// End of SNodePanel::SNode interface

	
	TSharedPtr<SHorizontalBox> OutNodeBox;

	
	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual TSharedPtr<SToolTip> GetComplexTooltip() override;
	// End of SGraphNode interface

protected:
	FSlateColor GetBorderBackgroundColor() const;

	virtual const FSlateBrush* GetNameIcon() const;
};
