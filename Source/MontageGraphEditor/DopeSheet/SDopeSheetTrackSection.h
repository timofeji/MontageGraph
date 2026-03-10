// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Tracks/DopeSheetTrackViewModel.h"
#include "Widgets/SCompoundWidget.h"

class FDopeSheetTrackViewModel;
class SDopeSheetTrackSection;

class SDopeSheetTrackSection_SizeHandle ;

UENUM()
enum class EDopeSheetTrackSectionHandleType : uint8
{
	/** Setup Event */
	Start,

	/** Update Event */
	End,

	/** None - invalid */
	None UMETA(Hidden),
};



class MONTAGEGRAPHEDITOR_API SDopeSheetTrackSection_SizeHandle : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDopeSheetTrackSection_SizeHandle)
		{
		}

	SLATE_END_ARGS()


	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetSectionViewModel> InSectionModel, EDopeSheetTrackSectionHandleType InType);
		

	bool                                   bIsDragged = false;

private:
	TSharedPtr<FDopeSheetSectionViewModel> SectionModel;
	const FSlateBrush*                     ResizeHandleBrush;
	const FSlateBrush*                     ResizeHandleBrush_Hovered;
	EDopeSheetTrackSectionHandleType       HandleType;
	bool                                   bIsHovered = false;
};


struct FDopeSheetSectionDragDrop_Resize : public FDragDropOperation
{
	DRAG_DROP_OPERATOR_TYPE(FDopeSheetSectionDragDrop , FDragDropOperation);
	
	virtual void Construct() override;

	virtual void OnDrop( bool bDropWasHandled, const FPointerEvent& MouseEvent ) override;

	virtual void OnDragged( const class FDragDropEvent& DragDropEvent ) override;

	static TSharedRef<FDopeSheetSectionDragDrop_Resize> New(
		TSharedPtr<SDopeSheetTrackSection_SizeHandle> SectionWidget,
		TSharedPtr<FDopeSheetSectionViewModel>&       InSectionModel,
		const FVector2D&                              CursorPosition,
		const EDopeSheetTrackSectionHandleType        HandleType);
		

	/** Gets the widget that will serve as the decorator unless overridden. If you do not override, you will have no decorator */
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	friend class SDopeSheetTrack;
	friend class SDopeSheetSection;

protected:
	TWeakPtr<SDopeSheetTrackSection_SizeHandle >     HandleWidget;
	TWeakPtr<FDopeSheetSectionViewModel> SectionModel;
	FVector2D                            StartingScreenPos;
	EDopeSheetTrackSectionHandleType     HandleType;
	

	FString DecoratorString;
	
	double     InitialTime;

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
	void Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetTrackViewModel> TrackModel, int SectionIndex);


	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

protected:
	bool bIsHovered;
	TSharedPtr<FDopeSheetSectionViewModel>  SectionModel;

private:
	FLinearColor SequenceSectionColor;
	
	const FSlateBrush* SectionBodyBrush;
	const FSlateBrush* SectionBodyBrush_Hovered;
	const FSlateBrush* SectionBorderBrush;
	const FSlateBrush* SectionBorderBrush_Selected;
	
};
