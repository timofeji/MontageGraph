//Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UMontageGraphNode;

/**
 * Delegate fired to get the path to start picking from.
 * @return the path the picker will use to start from
 */
DECLARE_DELEGATE_RetVal(FText, FACEOnGetValueText);

/** Draws a combo node property row to render label / values for a given property */
class MONTAGEGRAPHEDITOR_API SHBMontageGraphNodeRow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHBMontageGraphNodeRow) {}

	SLATE_ARGUMENT(FText, LabelText)
	SLATE_ARGUMENT(FText, LabelTooltipText)
	SLATE_ARGUMENT(FText, ValueTooltipText)
	SLATE_ARGUMENT(bool, IsTransparent)

	/** Delegate fired to get the actual value from GetValueText */
	SLATE_ARGUMENT(FACEOnGetValueText, OnGetValueText)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UMontageGraphNode* InRuntimeNode);

protected:

	UMontageGraphNode* RuntimeNode = nullptr;
	FText LabelText;
	FText LabelTooltipText;
	FText ValueTooltipText;
	bool bIsTransparent = false;

	FACEOnGetValueText OnGetValueText;

	FText GetValueText() const;
};
