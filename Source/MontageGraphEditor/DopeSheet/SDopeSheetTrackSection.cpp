// Copyright Drop Games Inc.


#include "SDopeSheetTrackSection.h"

#include "MontageGraphEditorStyle.h"
#include "SlateOptMacros.h"
#include "Tracks/DopeSheetTrackBase.h"
#include "Tracks/DopeSheetTrackViewModel.h"

//
TSharedPtr<SWidget> FDopeSheetSectionDragDrop_Resize::GetDefaultDecorator() const
{

	return SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		[
			SNew(STextBlock)
			.Text_Lambda([this] {
				return FText::FromString(DecoratorString);
			})
		];
}

void FDopeSheetSectionDragDrop_Resize::Construct()
{
	FDragDropOperation::Construct();

	// CursorDecoratorWindow->SetOpacity(.25f);
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

	
	HandleWidget.Pin()->bIsDragged = false;
	
	if (SectionModel.IsValid())
	{
		SectionModel.Pin()->CommitTimeRange();
	}


	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}


void FDopeSheetSectionDragDrop_Resize::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	// FSlateRect BoundingRect = SectionModel.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
	// FSlateRect SectionRect  = SectionBeingDragged.Pin()->GetCachedGeometry().GetLayoutBoundingRect();

	TSharedPtr<FDopeSheetSectionViewModel> SectionModelPtr = SectionModel.Pin();
	if (!SectionModelPtr)
	{
		return;
	}
	//
	FVector2D   MousePos  = DragDropEvent.GetScreenSpacePosition();
	const float deltaTime = SectionModelPtr->TrackModelPtr->Controller->AbsoluteXCoordToTime(MousePos.X) - SectionModelPtr->TrackModelPtr->Controller->AbsoluteXCoordToTime(StartingScreenPos.X);
	
	HandleType == EDopeSheetTrackSectionHandleType::Start ? SectionModel.Pin()->StartTime = InitialTime + deltaTime : SectionModel.Pin()->EndTime = InitialTime + deltaTime;
	
	DecoratorString = FString::Printf(TEXT("%.2f %.2f"), InitialTime, deltaTime);

	if (HandleWidget.IsValid())
	{
		CursorDecoratorWindow->MoveWindowTo(HandleWidget.Pin()->GetCachedGeometry().AbsolutePosition - CursorDecoratorWindow->GetCachedGeometry().GetAbsoluteSize());
	}
}

TSharedRef<FDopeSheetSectionDragDrop_Resize> FDopeSheetSectionDragDrop_Resize::New(
	TSharedPtr<SDopeSheetTrackSection_SizeHandle> InHandleWidget,
	TSharedPtr<FDopeSheetSectionViewModel>&       InSectionModel,
	const FVector2D&                              CursorPosition,
	const EDopeSheetTrackSectionHandleType        InHandleType)
{
	TSharedRef<FDopeSheetSectionDragDrop_Resize> Operation = MakeShareable(new FDopeSheetSectionDragDrop_Resize);

	Operation->HandleWidget      = InHandleWidget;
	Operation->SectionModel      = InSectionModel;
	Operation->StartingScreenPos = CursorPosition;
	Operation->HandleType = InHandleType;

	Operation->InitialTime = (InHandleType == EDopeSheetTrackSectionHandleType::Start) ? InSectionModel->StartTime : InSectionModel->EndTime;

	Operation->Construct();
	
	InHandleWidget->bIsDragged = true;


	return Operation;
}


void SDopeSheetTrackSection_SizeHandle::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) 
{
	SetCursor(EMouseCursor::ResizeLeftRight);
	bIsHovered = true;
}

void SDopeSheetTrackSection_SizeHandle::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SetCursor(EMouseCursor::Default);
	bIsHovered = false;
}

FReply SDopeSheetTrackSection_SizeHandle::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FVector2D ScreenCursorPos = MouseEvent.GetScreenSpacePosition();

		return FReply::Handled().BeginDragDrop(
			FDopeSheetSectionDragDrop_Resize::New(
				SharedThis(this),
				SectionModel,
				ScreenCursorPos,
				HandleType)).ReleaseMouseCapture();
	}
	//
	return FReply::Unhandled();
};

void SDopeSheetTrackSection_SizeHandle::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetSectionViewModel> InSectionModel, EDopeSheetTrackSectionHandleType InType)
{
	ResizeHandleBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.ResizeHandle");
	ResizeHandleBrush_Hovered = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.ResizeHandle.Hovered");

	HandleType   = InType;
	SectionModel = InSectionModel;

	ChildSlot
	[
		SNew(SImage)
		.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		.RenderTransform_Lambda([this]() {
			const float BaseSize = (bIsHovered||bIsDragged) ? 2.f : 1.5f;
			return FSlateRenderTransform(FScale2D(HandleType == EDopeSheetTrackSectionHandleType::Start ? -BaseSize : BaseSize, 1.0f));
		})
		.DesiredSizeOverride(FVector2D(16.f))
		.OnMouseButtonDown_Lambda([this](const FGeometry&, const FPointerEvent&) {
			return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
		})
		.Image_Lambda([this]() {
			return (bIsHovered||bIsDragged) ? ResizeHandleBrush_Hovered : ResizeHandleBrush;
		})
	];
};

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDopeSheetTrackSection::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetTrackViewModel> InTrackModel, int SectionIndex)
{
	SectionBodyBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.Section.Body");
	SectionBodyBrush_Hovered = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.Section.Body.Hovered");
	SectionBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.Section.Border");
	SectionBorderBrush_Selected = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.Section.Border.Selected");


	SectionModel = InTrackModel->SectionModels[SectionIndex];
	SequenceSectionColor = InTrackModel->ObjPtr->GetSectionColor();

	SetCursor(EMouseCursor::GrabHand);
	ChildSlot
	[
		SNew(SBorder)
		.Padding(0)
		.BorderImage_Lambda([this] {
			const bool bIsSelected = SectionModel->IsSelected();
			return (bIsSelected) ? SectionBorderBrush_Selected : SectionBorderBrush;
		})
		.BorderBackgroundColor_Lambda([this] {
			const bool bIsSelected = SectionModel->IsSelected();
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
				.Image_Lambda([this]() {
					return bIsHovered
							   ? SectionBodyBrush
							   : SectionBodyBrush_Hovered;
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
					SNew(SDopeSheetTrackSection_SizeHandle, SectionModel, EDopeSheetTrackSectionHandleType::Start)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				[
					SNew(SDopeSheetTrackSection_SizeHandle,SectionModel, EDopeSheetTrackSectionHandleType::End)
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

