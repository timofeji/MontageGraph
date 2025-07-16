// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SNodePanel.h"
#include "SMGNode.h"


class UMGEdNode_Entry;

class SMGNode_Entry : public SMGNode
{
public:
	SLATE_BEGIN_ARGS(SMGNode_Entry){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMGEdNode_Entry* InNode);

	// SNodePanel::SNode interface
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
	// End of SNodePanel::SNode interface


	virtual const bool IsTitleVisible() const override;

	// SGraphNode interface
	// virtual void UpdateGraphNode() override;
	// virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// virtual void CreatePinWidgets() override;
	//
	// End of SGraphNode interface

protected:
	FSlateColor GetBorderBackgroundColor() const;
	FText GetPreviewCornerText() const;
};
