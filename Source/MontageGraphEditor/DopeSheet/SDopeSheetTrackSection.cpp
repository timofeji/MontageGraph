// Copyright Drop Games Inc.


#include "SDopeSheetTrackSection.h"

#include "MontageGraphEditorStyle.h"
#include "SlateOptMacros.h"
#include "Tracks/DopeSheetTrackBase.h"
#include "Tracks/DopeSheetTrackViewModel.h"


TSharedPtr<SWidget> FDopeSheetSectionDragDrop_Resize::GetDefaultDecorator() const
{
	return SectionBeingDragged.Pin();
}

void FDopeSheetSectionDragDrop_Resize::Construct()
{
	FDragDropOperation::Construct();

	CursorDecoratorWindow->SetOpacity(.25f);
}

void FDopeSheetSectionDragDrop_Resize::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	// if ( bDropWasHandled == false )
	// {
	// 	if (SectionBeingDragged.IsValid())
	// 	{
	// 		SectionBeingDragged.Pin()->OnDropCancelled(MouseEvent);
	// 	}
	// }
	//

	if (OwningTrack.IsValid())
	{
		// OwningTrack.Pin()->OnRearrangeDrop();
	}


	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}


void FDopeSheetSectionDragDrop_Resize::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	FSlateRect BoundingRect = OwningTrack.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
	FSlateRect SectionRect  = SectionBeingDragged.Pin()->GetCachedGeometry().GetLayoutBoundingRect();


	FVector2D MousePos = DragDropEvent.GetScreenSpacePosition();
	FVector2D SectionDerotatowPos;
	SectionDerotatowPos.X = FMath::Clamp((MousePos + Offset).X,
	                                     BoundingRect.Left,
	                                     BoundingRect.Right - SectionRect.GetSize().X);
	SectionDerotatowPos.Y = StartingScreenPos.Y;
	CursorDecoratorWindow->MoveWindowTo(SectionDerotatowPos);

	// OwningTrack.Pin()->PreviewRearrange(MousePos);
}

TSharedRef<FDopeSheetSectionDragDrop_Resize> FDopeSheetSectionDragDrop_Resize::New(
	TSharedRef<SDopeSheetTrackSection> ParentTrack,
	TSharedRef<SWidget>          SectionWidget, const FVector2D& CursorPosition, const FVector2D& ScreenPositionOfNode)
{
	TSharedRef<FDopeSheetSectionDragDrop_Resize> Operation = MakeShareable(new FDopeSheetSectionDragDrop_Resize);

	Operation->OwningTrack         = ParentTrack;
	Operation->SectionBeingDragged = SectionWidget;
	Operation->Offset              = ScreenPositionOfNode - CursorPosition;
	Operation->StartingScreenPos   = ScreenPositionOfNode;

	Operation->Construct();


	return Operation;
}

class MONTAGEGRAPHEDITOR_API SDopeSheetTrackSection_SizeHandle : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDopeSheetTrackSection)
		{
		}

	SLATE_END_ARGS()

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		SetCursor(EMouseCursor::ResizeLeftRight);
		bIsHovered = true;
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		SetCursor(EMouseCursor::Default);
		bIsHovered = false;
	}
	
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return FReply::Unhandled();
	};

	

	void Construct(const FArguments& InArgs)
	{
		ResizeHandleBrush = FMontageGraphEditorStyle::Get().GetBrush(
			"MontageGraph.SequenceTrack.Section.ResizeHandle");
		ResizeHandleBrush_Hovered = FMontageGraphEditorStyle::Get().GetBrush(
			"MontageGraph.SequenceTrack.Section.ResizeHandle.Hovered");
		ChildSlot
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(32.f))
			.OnMouseButtonDown_Lambda([this](const FGeometry&, const FPointerEvent&)
			{
				return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
			})
			.Image_Lambda([this]()
			{
				return bIsHovered ? ResizeHandleBrush_Hovered : ResizeHandleBrush;
			})
		];
	};

private:
	const FSlateBrush* ResizeHandleBrush;
	const FSlateBrush* ResizeHandleBrush_Hovered;
	bool bIsHovered = false;
};

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDopeSheetTrackSection::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetTrackViewModel> TrackModel)
{
	SectionBodyBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Body");
	SectionBodyBrush_Hovered = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Body.Hovered");
	SectionBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Border");
	SectionBorderBrush_Selected = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Preview");


	SequenceSectionColor = TrackModel->ObjPtr->GetTrackColor();

	SetCursor(EMouseCursor::GrabHand);
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage_Lambda([this]
		{
			return (bIsSelected) ? SectionBorderBrush_Selected : SectionBorderBrush;
		})
		.BorderBackgroundColor_Lambda([this]
		{
			return (bIsSelected) ? FColor::White : SequenceSectionColor;
		})
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.ColorAndOpacity(SequenceSectionColor)
				.Image_Lambda([this]()
				{
					return bIsHovered
						       ? SectionBodyBrush_Hovered
						       : SectionBodyBrush;
				})
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					SNew(SDopeSheetTrackSection_SizeHandle)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				[
					SNew(SDopeSheetTrackSection_SizeHandle)
				]
			]
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SDopeSheetTrackSection ::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsHovered = true;
};

void SDopeSheetTrackSection ::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	bIsHovered = false;
};


FVector2D SDopeSheetTrackSection::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return GetCachedGeometry().Size;
}

