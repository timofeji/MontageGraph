#include "DopeSheetController.h"

#include "MontageGraphEditorStyle.h"
#include "Tracks/DopeSheetTrackBase.h"
#include "Tracks/DopeSheetTrackViewModel.h"

#if WITH_EDITOR

void FDopeSheetController::ForceUpdate()
{
	EDopeSheetFlags PreUpdateFlags = DirtyFlags;

	if (EnumHasAnyFlags(DirtyFlags, EDopeSheetFlags::VLayoutChanged | EDopeSheetFlags::RebuildTracks))
	{
		CachedTrackHeights.Reset(RootTracks.Num());

		for (int i = 0; i < VisibleTracks.Num(); i++)
		{
			UDopeSheetTrackBase* Track       = VisibleTracks[i];
			const float          TrackHeight = Track->GetTrackHeight();

			CachedTrackHeights.Add(TrackHeight);
		}
	}

	OnUpdate.Broadcast(DirtyFlags);

	//Remove Stale Flag
	GEditor->GetTimerManager()->SetTimerForNextTick([this]()
	{
		DirtyFlags = EDopeSheetFlags::None;
	});
}

void FDopeSheetController::AddDirtyFlags(EDopeSheetFlags NewFlags)
{
	DirtyFlags |= (NewFlags | EDopeSheetFlags::Stale);
}

void FDopeSheetController::SetTracksSource(const TArray<UDopeSheetTrackBase*>& TrackSrc)
{
	RootTracks.Reset(TrackSrc.Num());
	VisibleTracks.Reset(TrackSrc.Num());

	TSharedPtr<FDopeSheetController> ThisController = SharedThis(this);

	for (auto Track : TrackSrc)
	{
		FDopeSheetTrackViewModelRef NewTrackModel = MakeShared<FDopeSheetTrackViewModel>(Track, ThisController);
		RootTracks.Add(NewTrackModel);
		VisibleTracks.Add(Track);

		for (auto SubTrack : Track->SubTracks)
		{
			FDopeSheetTrackViewModelRef NewSubTrackModel = MakeShared<FDopeSheetTrackViewModel>(
				SubTrack, ThisController);
			NewTrackModel->Children.Add(NewSubTrackModel);
		}
	}

	AddDirtyFlags(EDopeSheetFlags::RebuildTracks);
	ForceUpdate();
}

void FDopeSheetController::OnTrackExpanded(TSharedRef<FDopeSheetTrackViewModel> ExpandedTrackModel, bool bExpanded)
{
	if (ExpandedTrackModel->ObjPtr)
	{
		ExpandedTrackModel->ObjPtr->bIsExpanded = bExpanded;

		for (auto Child : ExpandedTrackModel->Children)
		{
			bExpanded
				? VisibleTracks.AddUnique(Child->ObjPtr)
				: VisibleTracks.Remove(Child->ObjPtr);
		}
	}

	AddDirtyFlags(EDopeSheetFlags::VLayoutChanged);
	ForceUpdate();
}


void FDopeSheetController::AddViewSection(FDopeSheetViewSection NewSection)
{
	TotalNumberOfFrames += NewSection.NumOfFrames;
	Sections.Add(NewSection);
}

void FDopeSheetController::Reset(bool bMarkDirty)
{
	int DefaultNumFrames = 60;
	Sections.Reset(1);
	TotalNumberOfFrames = 0;

	if (bMarkDirty)
	{
		AddDirtyFlags(EDopeSheetFlags::All);
	}
}

FReply FDopeSheetController::OnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (HasSelectionFlags(EDopeSheetSelectionFlags::Selecting))
		{
			DragSelection(InGeometry, InMouseEvent);
		}
	}
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		// bIsPanning        = true;
		// const float Delta = (InMouseEvent.GetCursorDelta().X / InGeometry.GetAbsoluteSize().X) * TimeDurationInView;
		// SetViewRange(ViewStartTime - Delta, ViewEndTime - Delta);
	}

	return FReply::Handled();
}

void FDopeSheetController::CreateContextMenu(const FPointerEvent& MouseEvent, TSharedRef<SWidget> InWidget)
{
	// FMenuBuilder ContextMenuBuilder(true, nullptr);


	// VisibleTracks

	// Show the floating menu
	// FSlateApplication::Get().PushMenu(
	// 	InWidget,
	// 	MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath(),
	// 	ContextMenuBuilder.MakeWidget(),
	// 	MouseEvent.GetScreenSpacePosition(),
	// 	FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
	// );
}

FReply FDopeSheetController::OnMouseButtonUp(const FGeometry&    InGeometry, const FPointerEvent& MouseEvent,
                                             TSharedRef<SWidget> InWidget)
{
	FReply Reply = FReply::Unhandled();

	if ((HasSelectionFlags(EDopeSheetSelectionFlags::Selecting))
		&& MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		EndSelection();

		// Reply = FReply::Handled().ReleaseMouseCapture();
	}
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (bIsRMB_Pressed)
		{
			// if (bIsPanning)
			// {
			// 	bIsPanning = false;
			// 	InWidget->SetCursor(EMouseCursor::Default);
			// }
			//


			bIsRMB_Pressed = false;
			CreateContextMenu(MouseEvent, InWidget);
			Reply = FReply::Handled().ReleaseMouseCapture();
		}
	}

	return Reply;
}

FReply FDopeSheetController::OnMouseButtonDown(const FGeometry&    InGeometry, const FPointerEvent& InMouseEvent,
                                               TSharedRef<SWidget> InWidget)
{
	FReply Reply = FReply::Unhandled();
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SetSelectionFlags(EDopeSheetSelectionFlags::Selecting);
		BeginSelection(InGeometry, InMouseEvent);

		bIsLMB_Pressed = true;

		// Reply = FReply::Handled().CaptureMouse(InWidget);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bIsRMB_Pressed = true;
		// InWidget->SetCursor(EMouseCursor::CardinalCross);
		Reply = FReply::Handled().CaptureMouse(InWidget);
	}

	return Reply;
}

void FDopeSheetController::KeySelection()
{
	for (int i = SelectionStartRow; i <= SelectionEndRow; i++)
	{
		if (VisibleTracks.IsValidIndex(i))
		{
			UDopeSheetTrackBase* SelectedTrack = VisibleTracks[i];
			if (SelectedTrack)
			{
				float StartTime = SelectionStartFrame / (float)TotalNumberOfFrames;
				float EndTime   = SelectionEndFrame / (float)TotalNumberOfFrames;
				SelectedTrack->KeyTimeRange(StartTime, EndTime);
				SelectedTrack->OnTrackPropertiesChanged.Broadcast();

				ClearSelection();
			}
		}
	}
}

void FDopeSheetController::SetSelectionFlags(EDopeSheetSelectionFlags InFlags)
{
	SelectionFlags |= InFlags;
}

bool FDopeSheetController::HasSelectionFlags(EDopeSheetSelectionFlags FlagsToCheck) const
{
	return (SelectionFlags & FlagsToCheck) > EDopeSheetSelectionFlags::None;
}

void FDopeSheetController::BeginSelection(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FVector2f MouseLocationAbsolute = InMouseEvent.GetScreenSpacePosition();
	FVector2f MouseLocation         = InGeometry.AbsoluteToLocal(MouseLocationAbsolute);

	float AdjustedX = ((MouseLocation.X / InGeometry.Size.X) * TimeDurationInView) + ViewStartTime;

	StartCoord = FVector2D(AdjustedX, MouseLocation.Y);
	EndCoord   = StartCoord;

	SelectionStartRow = -1;
	SelectionEndRow   = -1;

	SelectionFlags &= ~EDopeSheetSelectionFlags::RangeSelected;
}

void FDopeSheetController::DragSelection(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FVector2f MouseLocationAbsolute = InMouseEvent.GetScreenSpacePosition();
	FVector2f MouseLocation         = InGeometry.AbsoluteToLocal(MouseLocationAbsolute);

	float AdjustedX = ((MouseLocation.X / InGeometry.Size.X) * TimeDurationInView) + ViewStartTime;
	EndCoord        = FVector2D(AdjustedX, MouseLocation.Y);


	FVector2D A = FVector2D(TimeToXOffset(StartCoord.X, InGeometry), StartCoord.Y);
	FVector2D B = FVector2D(TimeToXOffset(EndCoord.X, InGeometry), EndCoord.Y);

	const float StartTime = FMath::Min(AdjustedX, StartCoord.X);
	const float EndTime   = FMath::Max(AdjustedX, StartCoord.X);

	if (CachedSectionCellWidths.IsEmpty() || CachedTrackHeights.IsEmpty())
	{
		return;
	}


	int AccumulatedFrames = 0;
	for (int i = 0; i < Sections.Num(); ++i)
	{
		FDopeSheetViewSection ViewSection         = Sections[i];
		const int             NumOfFrames      = ViewSection.NumOfFrames;
		const float           CellViewDuration = (ViewSection.EndTime - ViewSection.StartTime);

		if (StartTime > ViewSection.StartTime && StartTime < ViewSection.EndTime)
		{
			const float Fraction = (StartTime - ViewSection.StartTime) / CellViewDuration;
			SelectionStartFrame  = FMath::Max(
				0, FMath::FloorToInt(Fraction * NumOfFrames) + AccumulatedFrames);
		}

		if (EndTime > ViewSection.StartTime && EndTime < ViewSection.EndTime)
		{
			const float Fraction = (EndTime - ViewSection.StartTime) / CellViewDuration;
			SelectionEndFrame    = FMath::Max(
				0, FMath::FloorToInt(Fraction * NumOfFrames) + AccumulatedFrames);
		}

		AccumulatedFrames += NumOfFrames;
	}


	FVector2D TopLeft(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
	FVector2D Size(FMath::Abs(A.X - B.X), FMath::Abs(A.Y - B.Y));

	float CurrentY = TimeSliderHeight.Get();

	// Find which rows the selection spans
	for (int i = 0; i < CachedTrackHeights.Num(); ++i)
	{
		float NextY = CurrentY + CachedTrackHeights[i];

		if (SelectionStartRow == -1 && TopLeft.Y < NextY)
		{
			SelectionStartRow = i;
		}

		if (TopLeft.Y + Size.Y <= NextY)
		{
			SelectionEndRow = i;
			break;
		}

		CurrentY = NextY;
	}

	
	SelectionEndRow = FMath::Clamp(SelectionEndRow, 0, CachedTrackHeights.Num() - 1);
}

void FDopeSheetController::EndSelection()
{
	if (HasSelectionFlags(EDopeSheetSelectionFlags::BoxSelect))
	{
		ProcessBoxSelection();
		SelectionFlags &= ~EDopeSheetSelectionFlags::BoxSelect;
	}
	else
	{
		SetSelectionFlags(EDopeSheetSelectionFlags::RangeSelected);
	}

	SelectionFlags &= ~EDopeSheetSelectionFlags::Selecting;
}

void FDopeSheetController::ClearSelection()
{
	SelectionFlags = EDopeSheetSelectionFlags::None;
}

void FDopeSheetController::ProcessBoxSelection()
{
}


FPaintGeometry FDopeSheetController::GetAdjustedSelectionGeometry(const FGeometry& InGeometry) const
{
	FVector2D A = FVector2D(TimeToXOffset(StartCoord.X, InGeometry), StartCoord.Y);
	FVector2D B = FVector2D(TimeToXOffset(EndCoord.X, InGeometry), EndCoord.Y);


	FVector2D TopLeft(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
	FVector2D Size(FMath::Abs(A.X - B.X), FMath::Abs(A.Y - B.Y));


	const bool bBoxSelect = HasSelectionFlags(EDopeSheetSelectionFlags::BoxSelect);
	if (bBoxSelect)
	{
		return InGeometry.ToPaintGeometry(TopLeft, Size);
	}

	// TopLeft = FVector2D(
	// 	FMath::Clamp(TopLeft.X, CachedViewRect.Left, CachedViewRect.Right),
	// 	FMath::Clamp(TopLeft.Y, CachedViewRect.Top, CachedViewRect.Bottom));
	//
	// Size = FVector2D(
	// 	FMath::Clamp(Size.X, CachedViewRect.Left, CachedViewRect.Right),
	// 	FMath::Clamp(Size.Y, CachedViewRect.Top, CachedViewRect.Bottom));
	//
	if (CachedTrackHeights.IsEmpty())
	{
		return InGeometry.ToPaintGeometry(TopLeft, Size);
	}


	float OffsetX        = TopLeft.X;
	float OffsetY        = TopLeft.Y;
	float SnappedHeight = Size.Y;

	if (SelectionStartRow >= 0 && SelectionEndRow >= 0 && SelectionStartRow <= SelectionEndRow)
	{
		OffsetY = TimeSliderHeight.Get();
		for (int i = 0; i < SelectionStartRow; ++i)
		{
			OffsetY += CachedTrackHeights[i];
		}

		SnappedHeight = 0.f;
		for (int i = SelectionStartRow; i <= SelectionEndRow; ++i)
		{
			SnappedHeight += CachedTrackHeights[i];
		}
	}
	

	int AccumulatedFrames = 0;
	for (int i = 0; i < Sections.Num(); ++i)
	{
		FDopeSheetViewSection ViewSection         = Sections[i];
		const float           FrameSize        = CachedSectionCellWidths[i];
		const int             NumOfFrames      = ViewSection.NumOfFrames;

		if (SelectionStartFrame > AccumulatedFrames  && SelectionStartFrame < AccumulatedFrames + NumOfFrames)
		{
			OffsetX = (SelectionStartFrame*FrameSize);
		}

		if (SelectionEndFrame > AccumulatedFrames  && SelectionEndFrame < AccumulatedFrames  + NumOfFrames)
		{

		}

		AccumulatedFrames += NumOfFrames;
	}


	return InGeometry.ToPaintGeometry(FVector2D(OffsetX, OffsetY), FVector2D(Size.X, SnappedHeight));
}

//////////////////////////////////////////////////////////////////////////
// Draw Selection Box
//////////////////////////////////////////////////////////////////////////
void FDopeSheetController::DrawSelection(FSlateWindowElementList& OutDrawElements, uint32 LayerId,
                                         const FGeometry&         AllottedGeometry) const
{
	if (HasSelectionFlags(EDopeSheetSelectionFlags::Selecting | EDopeSheetSelectionFlags::RangeSelected))
	{
		static const FSlateBrush* SelectionBoxBrush = FMontageGraphEditorStyle::Get().GetBrush(
			"MontageGraph.DopeSheet.Border.Selecting");
		static const FSlateBrush* SelectionCommittedBoxBrush = FMontageGraphEditorStyle::Get().GetBrush(
			"MontageGraph.DopeSheet.Border.Selected");


		const bool bIsSelectionRange = HasSelectionFlags(EDopeSheetSelectionFlags::RangeSelected);

		FLinearColor SelectionBoxColor = FLinearColor::White;
		SelectionBoxColor.A            = bIsSelectionRange ? 1.f : 0.75f;

		FPaintGeometry SelectionBoxGeometry = GetAdjustedSelectionGeometry(AllottedGeometry);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 2, // Draw above the cells
			SelectionBoxGeometry,
			bIsSelectionRange ? SelectionCommittedBoxBrush : SelectionBoxBrush,
			ESlateDrawEffect::None,
			SelectionBoxColor
		);

		static FSlateFontInfo SelectionFontInfo = FAppStyle::GetFontStyle("Graph.VectorEditableTextBox");


		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			SelectionBoxGeometry,
			FString::Printf(TEXT("[%i-%i]"),
			                SelectionStartFrame,
			                SelectionEndFrame),
			SelectionFontInfo,
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}
}


void FDopeSheetController::ZoomInView(float Delta, float Center)
{
	const float DeltaStartTime = (ViewStartTime - Center) * Delta;
	const float DeltaEndTime   = (ViewEndTime - Center) * Delta;

	SetViewRange(ViewStartTime + DeltaStartTime, ViewEndTime + DeltaEndTime);
}

void FDopeSheetController::ScrollView(float MoveDelta)
{
	SetViewRange(ViewStartTime + MoveDelta, ViewEndTime + MoveDelta);
}

void FDopeSheetController::SetViewRange(double NewStartTime, double NewEndTime)
{
	ViewStartTime = NewStartTime;
	ViewEndTime   = NewEndTime;

	TimeDurationInView = NewEndTime - NewStartTime;

	AddDirtyFlags(EDopeSheetFlags::HViewChanged);
	ForceUpdate();
}

void FDopeSheetController::FrameViewRange()
{
	const float EndTime = Sections.IsEmpty() ? 1 : Sections.Last().EndTime;
	SetViewRange(0.f, EndTime);
}

float FDopeSheetController::LocalXCoordToTime(float X) const
{
	return ((X / CachedGeometry.Size.X) * TimeDurationInView) + ViewStartTime;
}

float FDopeSheetController::AbsoluteXCoordToTime(float X) const
{
	return (((X - CachedGeometry.GetAbsolutePosition().X) / CachedGeometry.Size.X) * TimeDurationInView) + ViewStartTime;
}

float FDopeSheetController::TimeToXOffset(const float InTime, const FGeometry& AllottedGeometry) const
{
	return AllottedGeometry.Size.X * ((InTime - ViewStartTime) / TimeDurationInView);
}

float FDopeSheetController::ConvertXCoordToTime(const float XCoord, const FGeometry& AllottedGeometry) const
{
	return ((XCoord / AllottedGeometry.Size.X) * TimeDurationInView) + ViewStartTime;
}


void FDopeSheetController::UpdateViewForGeometry(const FGeometry& AllottedGeometry)
{
	//Prevent Render thread from using stale cache

	if (Sections.IsEmpty())
	{
		return;
	}
	
	CachedGeometry = AllottedGeometry;


	//Cachce Frame Widths for each ViewSection
	CachedSectionCellWidths.Reset(Sections.Num());
	for (FDopeSheetViewSection ViewSection : Sections)
	{
		const float SectionDuration = ViewSection.EndTime - ViewSection.StartTime;
		const float SectionFraction = SectionDuration / TimeDurationInView;
		const float AllotedWidth    = (CachedGeometry.Size.X * SectionFraction);
		const float FrameWidth      = AllotedWidth / ViewSection.NumOfFrames;

		CachedSectionCellWidths.Add(FrameWidth);
	}

	//
	FDopeSheetViewSection FirstCellSection = Sections[0];
	FDopeSheetViewSection LastCellSection  = Sections.Last();

	const double ViewLeftBound  = TimeToXOffset(FirstCellSection.StartTime, CachedGeometry);
	const double ViewRightBound = TimeToXOffset(LastCellSection.EndTime, CachedGeometry);

	EditableRect = FSlateRect(ViewLeftBound, TimeSliderHeight.Get(), ViewRightBound,
	                            CachedGeometry.Size.Y - TimeSliderHeight.Get());
}

bool FDopeSheetController::CanDraw()
{
	if (Sections.Num() != CachedSectionCellWidths.Num()) { return false; }

	if (EnumHasAnyFlags(DirtyFlags, EDopeSheetFlags::Stale))
	{
		ForceUpdate(); //@todo: ~tim probably should be done in a tick function
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Timeline/PlayHead Time
/////////////////////////////////////////////////////////////////////////////////////////
void FDopeSheetController::SetPlayHeadTime(float NewTime, bool bBroadcastChange)
{
	PlayHeadTime = NewTime;
	if (bBroadcastChange)
	{
		OnTimeChanged.Broadcast(PlayHeadTime);
	}
}


void FDopeSheetController::ScrubPlayback(float NewPlaybackTime)
{
	PlayHeadTime = NewPlaybackTime;
	OnTimeChanged.Broadcast(NewPlaybackTime);
}


void FDopeSheetController::SetPlaybackState(bool bNewState)
{
	bIsPlayingAnimation = bNewState;
	OnTogglePlayback.Broadcast(bIsPlayingAnimation);
}

void FDopeSheetController::HoldPlayback(bool bShouldPlay)
{
	if (bShouldPlay)
	{
		bPlayBackBeforeHoldState = bIsPlayingAnimation;
		bIsPlayingAnimation      = bShouldPlay;
	}
	else
	{
		bIsPlayingAnimation      = bPlayBackBeforeHoldState;
		bPlayBackBeforeHoldState = false;
	}


	OnTogglePlayback.Broadcast(bIsPlayingAnimation);
}

bool FDopeSheetController::ShouldScrubPlayback()
{
	return bPlayBackBeforeHoldState && !bIsPlayingAnimation;
}
#endif
