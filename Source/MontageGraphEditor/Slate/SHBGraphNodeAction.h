
#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UMontageGraphNode_Action;
class UHBMontageGraphNodeBase;
class UMontageGraphEdNode;

class MONTAGEGRAPHEDITOR_API SHBGraphNodeAction : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SHBGraphNodeAction) {}
	SLATE_ARGUMENT(FMargin, ContentPadding)
	SLATE_ARGUMENT(FMargin, ContentInternalPadding)
	SLATE_ARGUMENT(float, PinSize)
	SLATE_ARGUMENT(float, PinPadding)
	SLATE_ARGUMENT(bool, DrawVerticalPins)
	SLATE_ARGUMENT(bool, DrawHorizontalPins)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UMontageGraphEdNode* InNode);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual bool IsNameReadOnly() const override;

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);


	virtual FSlateColor GetBorderBackgroundColor() const;

	virtual EVisibility GetDragOverMarkerVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;

	/** Allow outside code (such as Blueprint Editor) to update error text */
	void SetErrorText(FText InErrorText);
	
protected:
	UMontageGraphEdNode* CachedGraphNode = nullptr;

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

	FText GetMontageText();
	TSharedPtr<SVerticalBox> CreateNodeContent();
	//
	// EVisibility GetGameplayEffectCostTextVisibility() const;
	// FText GetGameplayEffectCostText() const;;
	// FText GetMontagePlayRateText() const;
	//
	FText GetMontageStartSectionName() const;
	EVisibility GetMontageStartSectionNameVisibility() const;

	FText GetErrorText() const;
};

