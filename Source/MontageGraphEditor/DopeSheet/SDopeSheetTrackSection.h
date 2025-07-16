// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FDopeSheetTrackViewModel;
class SDopeSheetTrackSection;


struct FDopeSheetSectionDragDrop_Resize : public FDragDropOperation
{
	DRAG_DROP_OPERATOR_TYPE(FDopeSheetSectionDragDrop , FDragDropOperation);
	
	virtual void Construct() override;

	virtual void OnDrop( bool bDropWasHandled, const FPointerEvent& MouseEvent ) override;

	virtual void OnDragged( const class FDragDropEvent& DragDropEvent ) override;
	

	static TSharedRef<FDopeSheetSectionDragDrop_Resize> New(
		TSharedRef<SDopeSheetTrackSection> ParentTrack,
		TSharedRef<SWidget> SectionWidget,
		const FVector2D&    CursorPosition,
		const FVector2D&    ScreenPositionOfNode);

	/** Gets the widget that will serve as the decorator unless overridden. If you do not override, you will have no decorator */
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	friend class SDopeSheetTrack;
	friend class SDopeSheetSection;

protected:
	TWeakPtr<SDopeSheetTrackSection> OwningTrack;
	TWeakPtr<SWidget>          SectionBeingDragged;
	FVector2D                  Offset;
	FVector2D                  StartingScreenPos;
	
};

/**
 * 
 */
class MONTAGEGRAPHEDITOR_API SDopeSheetTrackSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDopeSheetTrackSection)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetTrackViewModel> TrackModel);


	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	bool bIsSelected = false;
protected:
	bool               bIsHovered;
	
private:
	FLinearColor SequenceSectionColor;
	
	const FSlateBrush* SectionBodyBrush;
	const FSlateBrush* SectionBodyBrush_Hovered;
	const FSlateBrush* SectionBorderBrush;
	const FSlateBrush* SectionBorderBrush_Selected;
};
