// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FDopeSheetTrackViewModel;
class SDopeSheetTrackSection;
class SDopeSheetTrackTimeline;

/** Implements drag and drop operation. */
struct FDopeSheetSectionDragDropOp : public FDragDropOperation
{
	DRAG_DROP_OPERATOR_TYPE(FDopeSheetSectionDragDrop , FDragDropOperation);
	
	virtual void Construct() override;

	virtual void OnDrop( bool bDropWasHandled, const FPointerEvent& MouseEvent ) override;

	virtual void OnDragged( const class FDragDropEvent& DragDropEvent ) override;


	static TSharedRef<FDopeSheetSectionDragDropOp> New(
		TSharedRef<SDopeSheetTrackTimeline>  ParentTrack,
		TSharedPtr<FDopeSheetTrackViewModel> TrackModel,
		TSharedRef<SWidget>                  SectionWidget,
		int32                                DraggedSectionIndex,
		const FVector2D&                     CursorPosition,
		const FVector2D&                     ScreenPositionOfNode);

	/** Gets the widget that will serve as the decorator unless overridden. If you do not override, you will have no decorator */
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	friend class SDopeSheetTrack;
	friend class SDopeSheetSection;

protected:
	TSharedPtr<FDopeSheetTrackViewModel> TrackModel;
	int32 SectionIndex = -1;
	TWeakPtr<SDopeSheetTrackTimeline> OwnerTrack;
	TWeakPtr<SWidget> SectionBeingDragged;
	FVector2D         Offset;
	FVector2D         StartingScreenPos;
	
};


/**
 * 
 */


class MONTAGEGRAPHEDITOR_API SDopeSheetTrackTimeline : public SPanel
{
public:
	SLATE_BEGIN_ARGS(SDopeSheetTrackTimeline)
		{
		}

	SLATE_END_ARGS()

	SDopeSheetTrackTimeline();
	~SDopeSheetTrackTimeline();

	void RegenerateSections();
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetTrackViewModel> InTrackModel);

	virtual bool PassesAssetReferenceFilter(const TArray<FAssetData>& Array) { return false; }


	virtual void MakeSectionContextMenu(FMenuBuilder& ContextMenuBuilder);

	/** SPanel */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void
	OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D  ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual FChildren* GetChildren() override;

	virtual FReply      OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply      OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void DropDraggedSection(int32 SectionIndex, double MoveDelta);

	int32 GetSectionIndexUnderCursor(const FPointerEvent& InMouseEvent);

protected:
	TSlotlessChildren<SWidget>           TrackSectionWidgets;
	TSharedPtr<FDopeSheetTrackViewModel> TrackModel;
	
	int32 SelectedSectionIndex = -1;

	bool bIsDraggingSection = false;
};


