// Copyright Drop Games Inc.


#include "SDopeSheetTrackTimeline.h"

#include "DopeSheetTrackBase.h"
#include "DopeSheetTrackViewModel.h"
#include "MontageGraphEditorStyle.h"
#include "SlateOptMacros.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/SDopeSheetTrackSection.h"


TSharedPtr<SWidget> FDopeSheetSectionDragDropOp::GetDefaultDecorator() const
{
	return SectionBeingDragged.Pin();
}

void FDopeSheetSectionDragDropOp::Construct()
{
	FDragDropOperation::Construct();

	CursorDecoratorWindow->SetOpacity(.25f);
}

void FDopeSheetSectionDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	if (OwnerTrack.IsValid() && TrackModel.IsValid())
	{
		FSlateRect BoundingRect = OwnerTrack.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
		FSlateRect SectionRect  = SectionBeingDragged.Pin()->GetCachedGeometry().GetLayoutBoundingRect();


		FVector2D MousePos = MouseEvent.GetScreenSpacePosition();
		FVector2D DroppedPosition;
		DroppedPosition.X = FMath::Clamp((MousePos).X,
		                                 BoundingRect.Left,
		                                 BoundingRect.Right - SectionRect.GetSize().X);

		double endTime   = TrackModel->Controller->AbsoluteXCoordToTime(DroppedPosition.X);
		double startTime = TrackModel->Controller->AbsoluteXCoordToTime((StartingScreenPos).X);

		double MoveDelta = endTime - startTime;
		OwnerTrack.Pin()->DropDraggedSection(SectionIndex, MoveDelta);
	}


	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}


void FDopeSheetSectionDragDropOp::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	FSlateRect BoundingRect = OwnerTrack.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
	FSlateRect SectionRect  = SectionBeingDragged.Pin()->GetCachedGeometry().GetLayoutBoundingRect();


	FVector2D MousePos = DragDropEvent.GetScreenSpacePosition();
	FVector2D SectionDerotatowPos;
	SectionDerotatowPos.X = FMath::Clamp((MousePos + Offset).X,
	                                     BoundingRect.Left,
	                                     BoundingRect.Right - SectionRect.GetSize().X);
	SectionDerotatowPos.Y = StartingScreenPos.Y;
	CursorDecoratorWindow->MoveWindowTo(SectionDerotatowPos);
}

TSharedRef<FDopeSheetSectionDragDropOp> FDopeSheetSectionDragDropOp::New(
	TSharedRef<SDopeSheetTrackTimeline>  ParentTrack,
	TSharedPtr<FDopeSheetTrackViewModel> TrackModel,
	TSharedRef<SWidget>                  SectionWidget,
	int32                                DraggedSectionIndex,
	const FVector2D&                     CursorPosition,
	const FVector2D&                     ScreenPositionOfNode)
{
	TSharedRef<FDopeSheetSectionDragDropOp> Operation = MakeShareable(new FDopeSheetSectionDragDropOp);


	Operation->OwnerTrack          = ParentTrack;
	Operation->SectionBeingDragged = SectionWidget;
	Operation->SectionIndex        = DraggedSectionIndex;
	Operation->TrackModel          = TrackModel;
	Operation->Offset              = ScreenPositionOfNode - CursorPosition;
	Operation->StartingScreenPos   = ScreenPositionOfNode;

	Operation->Construct();


	return Operation;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDopeSheetTrackTimeline::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetTrackViewModel> InTrackModel)
{
	TrackModel = InTrackModel;

	if (TrackModel.IsValid() && TrackModel->ObjPtr)
	{
		TrackModel->ObjPtr->OnTrackPropertiesChanged.AddRaw(
			this, &SDopeSheetTrackTimeline::RegenerateSections);
	}


	RegenerateSections();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SDopeSheetTrackTimeline::SDopeSheetTrackTimeline()
	: TrackSectionWidgets(this)
{
}

SDopeSheetTrackTimeline::~SDopeSheetTrackTimeline()
{
	if (TrackModel.IsValid() && TrackModel->ObjPtr)
	{
		TrackModel->ObjPtr->OnTrackPropertiesChanged.RemoveAll(this);
	}
}

void SDopeSheetTrackTimeline::RegenerateSections()
{
	if ((TrackModel != nullptr) && (TrackModel->ObjPtr))
	{
		if (UDopeSheetTrackBase* Track = (TrackModel->ObjPtr))
		{
			const int NumOfSections = Track->Sections.Num();
			TrackSectionWidgets.Reset(NumOfSections);

			for (int i = 0; i < NumOfSections; i++)
			{
				if (UDopeSheetTrackSection* Section = Track->Sections[i])
				{
					TrackSectionWidgets.Add(SNew(SDopeSheetTrackSection, TrackModel));
				}
			}
		}
	}
}


void SDopeSheetTrackTimeline::MakeSectionContextMenu(FMenuBuilder& ContextMenuBuilder)
{
	if (!(TrackModel) || !(TrackModel->ObjPtr))
	{
		return;
	}

	FUIAction ItemAction(FExecuteAction::CreateLambda([this]()
	{
		TrackModel->DeleteSection(SelectedSectionIndex);
	}));
	ContextMenuBuilder.AddMenuEntry(FText::FromString("Delete Section"), TAttribute<FText>(),
	                                FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage"),
	                                ItemAction);
}

int32 SDopeSheetTrackTimeline::GetSectionIndexUnderCursor(const FPointerEvent& InMouseEvent)
{
	int32 ReturnIndex = -1;

	FVector2D ScreenCursorPos = InMouseEvent.GetScreenSpacePosition();
	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		const TSharedRef<SWidget> ChildWidget = TrackSectionWidgets[ChildIndex];

		const EVisibility ChildVisibility = ChildWidget->GetVisibility();
		if (ChildVisibility != EVisibility::Visible) { continue; }

		FSlateRect SectionRect = ChildWidget->GetCachedGeometry().GetRenderBoundingRect();
		if (SectionRect.Left < ScreenCursorPos.X && SectionRect.Right > ScreenCursorPos.X)
		{
			ReturnIndex = ChildIndex;
			break;
		}
	}
	return ReturnIndex;
}

FReply SDopeSheetTrackTimeline::OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = FReply::Unhandled();

	if (SelectedSectionIndex >= 0)
	{
		const TSharedRef<SDopeSheetTrackSection> SelectedSectionWidget = StaticCastSharedRef<SDopeSheetTrackSection>(
			TrackSectionWidgets[SelectedSectionIndex]);

		SelectedSectionWidget->bIsSelected = false;
		SelectedSectionIndex               = -1;
	}

	int32 SectionToSelectIndex = GetSectionIndexUnderCursor(InMouseEvent);
	if (SectionToSelectIndex >= 0)
	{
		const TSharedRef<SDopeSheetTrackSection> SectionToSelectWidget = StaticCastSharedRef<SDopeSheetTrackSection>(
			TrackSectionWidgets[SectionToSelectIndex]);


		if (UObject* SelectedSection = TrackModel->ObjPtr->Sections[SectionToSelectIndex])
		{
			TrackModel->Controller->OnSectionSelected.Broadcast(SelectedSection);
		}


		SectionToSelectWidget->bIsSelected = true;

		SelectedSectionIndex = SectionToSelectIndex;

		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			Reply = FReply::Handled().CaptureMouse(SharedThis(this));
		}

		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			Reply = FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
		}
	}
	return Reply;
}


FReply SDopeSheetTrackTimeline::OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = FReply::Unhandled().ReleaseMouseCapture();

	int32 SelectedIndex = GetSectionIndexUnderCursor(InMouseEvent);
	if (SelectedIndex >= 0)
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			FMenuBuilder ContextMenuBuilder(true, nullptr);
			MakeSectionContextMenu(ContextMenuBuilder);


			// Show the floating menu
			FSlateApplication::Get().PushMenu(
				FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef(),
				FWidgetPath(),
				ContextMenuBuilder.MakeWidget(),
				InMouseEvent.GetScreenSpacePosition(),
				FPopupTransitionEffect::ContextMenu
			);
			
			Reply = FReply::Handled().ReleaseMouseCapture();
		}
	}

	return Reply;
}

FReply SDopeSheetTrackTimeline::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		int32 SectionToSelectIndex = GetSectionIndexUnderCursor(MouseEvent);
		if (SectionToSelectIndex >= 0)
		{
			const TSharedRef<SDopeSheetTrackSection> SectionToDragWidget = StaticCastSharedRef<
				SDopeSheetTrackSection>(TrackSectionWidgets[SectionToSelectIndex]);

			FVector2D ScreenCursorPos = MouseEvent.GetScreenSpacePosition();
			FVector2D SectionOffset   = FVector2D(SectionToDragWidget->GetCachedGeometry().GetAbsolutePosition());


			bIsDraggingSection = true;

			return FReply::Handled().BeginDragDrop(
				FDopeSheetSectionDragDropOp::New(SharedThis(this),
				                                 TrackModel,
				                                 SectionToDragWidget,
				                                 SectionToSelectIndex,
				                                 ScreenCursorPos,
				                                 SectionOffset)).ReleaseMouseCapture();
		}
	}

	return SPanel::OnDragDetected(MyGeometry, MouseEvent);
}

void SDopeSheetTrackTimeline::DropDraggedSection(int32 SectionIndex, double MoveDelta)
{
	TrackModel->ShiftSection(SectionIndex, MoveDelta);
	bIsDraggingSection = false;
}


void SDopeSheetTrackTimeline::OnArrangeChildren(const FGeometry&   AllottedGeometry,
                                                FArrangedChildren& ArrangedChildren) const
{
	if (!TrackModel)
	{
		return;
	}

	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		const TSharedRef<SWidget> ChildWidget = TrackSectionWidgets[ChildIndex];

		const EVisibility ChildVisibility = ChildWidget->GetVisibility();
		if (!ArrangedChildren.Accepts(ChildVisibility) || !TrackModel->SectionModels.IsValidIndex(ChildIndex))
		{
			continue;
		}

		const float AllottedWidth  = static_cast<float>(AllottedGeometry.GetLocalSize().X);
		const float AllottedHeight = static_cast<float>(AllottedGeometry.GetLocalSize().Y);

		auto Section = TrackModel->SectionModels[ChildIndex];

		const float ViewedTimeCoefficient = Section.GetTimeLength() / TrackModel->Controller->TimeDurationInView;
		const float SectionWidth          = AllottedWidth * ViewedTimeCoefficient;

		const float SectionOffset = TrackModel->Controller->EditableRect.Left + TrackModel->Controller->EditableRect
			.GetSize().X * Section.StartTime;


		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
			ChildWidget,
			FVector2D(SectionOffset, 0),
			FVector2D(SectionWidth, AllottedHeight)));
	}
}

FVector2D SDopeSheetTrackTimeline::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(500.0f, 32.0f);
}

FChildren* SDopeSheetTrackTimeline::GetChildren()
{
	return &TrackSectionWidgets;
}

int32 SDopeSheetTrackTimeline::OnPaint(const FPaintArgs&   Args, const FGeometry&                  AllottedGeometry,
                                       const FSlateRect&   MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                       int32               InLayerId,
                                       const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (bIsDraggingSection && SelectedSectionIndex >= 0 && TrackModel)
	{
		auto                      SectionWidget      = TrackSectionWidgets[SelectedSectionIndex];
		static const FSlateBrush* PreviewBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
			"MontageGraph.SequenceTrack.Section.Preview");

		FGeometry DraggedSection = SectionWidget->GetPaintSpaceGeometry();
		FVector2f LocalPosition = FVector2f(DraggedSection.Position.X,DraggedSection.Position.Y);
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			InLayerId++,
			AllottedGeometry.ToPaintGeometry(DraggedSection.GetLocalSize(),
			                                 FSlateLayoutTransform(LocalPosition)),
			PreviewBorderBrush,
			ESlateDrawEffect::None,
			TrackModel->GetTrackColor()
		);


		return InLayerId;
	}

	return SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, InLayerId, InWidgetStyle,
	                       bParentEnabled);
}
