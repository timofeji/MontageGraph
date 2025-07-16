
#pragma once

#include "CoreMinimal.h"
#include "SMGNode.h"

class UMGNode_Montage;
class UMontageGraphNodeBase;
class UMGEdNode;

class MONTAGEGRAPHEDITOR_API SMGNode_Montage : public SMGNode
{
public:
	SLATE_BEGIN_ARGS(SMGNode_Montage) {}
	SLATE_ARGUMENT(FMargin, ContentPadding)
	SLATE_ARGUMENT(FMargin, ContentInternalPadding)
	SLATE_ARGUMENT(float, PinSize)
	SLATE_ARGUMENT(float, PinPadding)
	SLATE_ARGUMENT(bool, DrawVerticalPins)
	SLATE_ARGUMENT(bool, DrawHorizontalPins)
	SLATE_END_ARGS()
	
	// TSharedRef<SGraphPin> BlendInPin;
	
//
	void Construct(const FArguments& InArgs, UMGEdNode* InNode);


	virtual const bool IsTitleVisible() const override;

	// virtual TSharedRef<SWidget> CreateNodeContentArea() override;
// 	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
//
	// virtual void UpdateGraphNode() override;
// 	virtual void CreatePinWidgets() override;
// 	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
// 	virtual bool IsNameReadOnly() const override;
//
// 	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
//
// 	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);
//
//
// 	virtual FSlateColor GetBorderBackgroundColor() const;
//
// 	virtual EVisibility GetDragOverMarkerVisibility() const;
//
// 	virtual const FSlateBrush* GetNameIcon() const;
//
// 	/** Allow outside code (such as Blueprint Editor) to update error text */
// 	void SetErrorText(FText InErrorText);
// 	
// private:
//
// 	/** The node body widget, cached here so we can determine its size when we want ot position our overlays */
// 	TSharedPtr<SBorder> NodeBody;
//
// 	/** Custom Error handling widget */
// 	TSharedPtr<class IErrorReportingWidget> CustomErrorReporting;
//
// 	FMargin ContentPadding;
// 	FMargin ContentInternalPadding;
// 	float PinSize = 10.f;
// 	float PinPadding = 10.f;
// 	bool bDrawVerticalPins = true;
// 	bool bDrawHorizontalPins = true;
//
// 	FText GetMontageText();
	// TSharedPtr<SVerticalBox> CreateNodeContent();
// 	//
// 	// EVisibility GetGameplayEffectCostTextVisibility() const;
// 	// FText GetGameplayEffectCostText() const;;
// 	// FText GetMontagePlayRateText() const;
// 	//
// 	FText GetMontageStartSectionName() const;
// 	EVisibility GetMontageStartSectionNameVisibility() const;
//
// 	FText GetErrorText() const;
};

