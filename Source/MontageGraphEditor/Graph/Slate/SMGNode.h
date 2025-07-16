// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UMGEdNode;
/**
 * 
 */
class MONTAGEGRAPHEDITOR_API SMGNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SMGNode) {}
	SLATE_ARGUMENT(FMargin, ContentPadding)
	SLATE_ARGUMENT(FMargin, ContentInternalPadding)
	SLATE_ARGUMENT(float, PinSize)
	SLATE_ARGUMENT(float, PinPadding)
	SLATE_ARGUMENT(bool, DrawVerticalPins)
	SLATE_ARGUMENT(bool, DrawHorizontalPins)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMGEdNode* InNode);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// End of SGraphNode interface
	
	virtual bool IsNameReadOnly() const override;
	virtual const FSlateBrush* GetShadowBrush(bool bSelected) const override;
	virtual const FSlateBrush* GetNodeBodyBrush() const override;
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;


	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	
	FSlateColor GetNodeTitleColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;
	
	/** Allow outside code (such as Blueprint Editor) to update error text */
	void SetErrorText(FText InErrorText);
	
	virtual const bool IsTitleVisible() const;
	
	virtual bool UseLowDetailNode() const;

private:

	/** The node body widget, cached here so we can determine its size when we want ot position our overlays */
	TSharedPtr<SBorder> NodeBody;

	/** Custom Error handling widget */
	TSharedPtr<class IErrorReportingWidget> CustomErrorReporting;

	FMargin ContentPadding;
	FMargin ContentInternalPadding;
	float PinSize = 10.f;
	float PinPadding = 10.f;
	bool bDrawVerticalPins = true;
	bool bDrawHorizontalPins = true;

	FText GetErrorText() const;
};
