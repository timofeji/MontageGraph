// Created by Timofej Jermolaev, All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SGraphPin.h"

class SMontageGraphSelectorOutputPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SMontageGraphSelectorOutputPin){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	
	FGameplayTag SelectorTag;
	int32 TagIndex;
protected:
	// Begin SGraphPin interface
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

	virtual TSharedRef<FDragDropOperation> SpawnPinDragEvent(const TSharedRef<SGraphPanel>& InGraphPanel, const TArray<TSharedRef<SGraphPin>>& InStartingPins) override;
	// End SGraphPin interface

	const FSlateBrush* GetPinBorder() const;
	
};
