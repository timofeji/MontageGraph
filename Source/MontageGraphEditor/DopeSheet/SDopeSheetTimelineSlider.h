// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FDopeSheetController;


struct FDopeSheetTimelineSliderScrubOp : public FDragDropOperation
{
	DRAG_DROP_OPERATOR_TYPE(FDopeSheetSectionDragDrop , FDragDropOperation);
	
	virtual void Construct() override;

	virtual void OnDrop( bool bDropWasHandled, const FPointerEvent& MouseEvent ) override;

	virtual void OnDragged( const class FDragDropEvent& DragDropEvent ) override;
	

	static TSharedRef<FDopeSheetTimelineSliderScrubOp> New(
		TSharedPtr<FDopeSheetController> Controller,
		TSharedRef<SWidget> WidgetBeingDragged,
		const FVector2D&    CursorPosition,
		const FVector2D&    ScreenPositionOfNode);

	/** Gets the widget that will serve as the decorator unless overridden. If you do not override, you will have no decorator */
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	friend class SDopeSheetTrack;
	friend class SDopeSheetSection;

protected:
	TWeakPtr<FDopeSheetController> Controller;
	TWeakPtr<SWidget>          WidgetBeingDragged;
	FVector2D                  Offset;
	FVector2D                  StartingScreenPos;
	
};


/**
 * 
 */
class MONTAGEGRAPHEDITOR_API SDopeSheetTimelineSlider : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDopeSheetTimelineSlider)
		{
		}

		// The currently active mode, used to determine which mode is highlighted
	SLATE_END_ARGS()

	TSharedPtr<FDopeSheetController> Controller;
	const FSlateBrush*               BackgroundBrush;
	const FSlateBrush*               BackgroundBrush_Active;
	const FSlateBrush*               BackgroundBrush_Inactive;
	const FSlateBrush*               EndCellBrush;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController);
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;


	float GetScreenSpaceXOffsetFromTime(const float InTime, const FGeometry& AllottedGeometry) const;

};
