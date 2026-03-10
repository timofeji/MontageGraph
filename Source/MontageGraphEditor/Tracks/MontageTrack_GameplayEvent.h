#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"
#include "MontageTrack_GameplayEvent.generated.h"

class FDopeSheetController;

#if WITH_EDITOR

// ============================================================
// Section data — a single-frame point keyframe
// ============================================================

UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_GameplayEvent : public UDopeSheetTrackSection
{
	GENERATED_BODY()
public:
	/** Tag sent as the gameplay event when this keyframe fires. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayTag EventTag;
};

// ============================================================
// Drag-drop op — moves a keyframe horizontally
// ============================================================

struct MONTAGEGRAPHEDITOR_API FMGKeyframeDragDropOp : public FDragDropOperation
{
	DRAG_DROP_OPERATOR_TYPE(FMGKeyframeDragDropOp, FDragDropOperation)

	static TSharedRef<FMGKeyframeDragDropOp> New(
		TWeakObjectPtr<UMontageTrackSection_GameplayEvent> InSection,
		TSharedPtr<FDopeSheetController>                  InController,
		const FVector2D&                                  InStartPos);

	virtual void Construct() override;
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
	virtual void OnDragged(const FDragDropEvent& DragDropEvent) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	TWeakObjectPtr<UMontageTrackSection_GameplayEvent> SectionPtr;
	TSharedPtr<FDopeSheetController>                   ControllerPtr;
	FVector2D                                          StartScreenPos;
	double                                             InitialTime = 0.0;
};

// ============================================================
// Custom timeline widget — draws diamond icons for each keyframe
// ============================================================

class MONTAGEGRAPHEDITOR_API SMGGameplayEventTrack : public SDopeSheetTrackTimeline
{
public:
	SLATE_BEGIN_ARGS(SMGGameplayEventTrack) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                      const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	                      int32 InLayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void MakeTrackContextMenu(FMenuBuilder& MenuBuilder) override;
	virtual void MakeSectionContextMenu(FMenuBuilder& MenuBuilder) override;

private:
	/** Returns section index under LocalPos (±HitRadius), or -1. */
	int32 HitTestKeyframe(const FGeometry& Geometry, const FVector2D& LocalPos) const;

	double LastRightClickTime   = 0.0;
	int32  ContextKeyframeIndex = -1;
	int32  DragKeyframeIndex    = -1;
};

// ============================================================
// Track class
// ============================================================

UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_GameplayEvent : public UDopeSheetTrackBase
{
	GENERATED_BODY()
public:
	UMontageTrack_GameplayEvent();

	virtual void BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
	                        UMontageGraph* OwnerGraph, const FString& DisplayName) override;

	virtual const FText        GetTrackName()    const override;
	virtual const FLinearColor GetTrackColor()   const override;
	virtual const FLinearColor GetSectionColor() const override;
	virtual FSlateIcon         GetTrackIcon()    const override;

	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(
		TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const override;
};

#endif
