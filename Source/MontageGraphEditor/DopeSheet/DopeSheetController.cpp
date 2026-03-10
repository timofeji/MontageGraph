#include "DopeSheetController.h"

#include "MontageGraphEditorStyle.h"
#include "ToolContextInterfaces.h"
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

void FDopeSheetController::SetTracksSource(TArray<UDopeSheetTrackBase*>& TrackSrc, UObject* TracksOuter)
{
	RootTracks.Reset(TrackSrc.Num());
	VisibleTracks.Reset(TrackSrc.Num());

	ClearSelection();

	TracksOwner = TracksOuter;
	TracksPtr   = &TrackSrc;

	// Register any new collection names as visible by default
	for (UDopeSheetTrackBase* Track : TrackSrc)
	{
		if (Track && Track->CollectionName != NAME_None && !CollectionVisibility.Contains(Track->CollectionName))
		{
			CollectionVisibility.Add(Track->CollectionName, true);
		}
	}

	TSharedPtr<FDopeSheetController> ThisController = SharedThis(this);

	for (UDopeSheetTrackBase* Track : TrackSrc)
	{
		if (!Track) { continue; }
		if (!IsCollectionVisible(Track->CollectionName)) { continue; }

		FDopeSheetTrackViewModelRef NewTrackModel = MakeShared<FDopeSheetTrackViewModel>(Track, ThisController);
		RootTracks.Add(NewTrackModel);
		VisibleTracks.Add(Track);

		for (UDopeSheetTrackBase* SubTrack : Track->SubTracks)
		{
			FDopeSheetTrackViewModelRef NewSubTrackModel = MakeShared<FDopeSheetTrackViewModel>(
				SubTrack, ThisController);
			NewTrackModel->Children.Add(NewSubTrackModel);
			NewSubTrackModel->ParentPtr = Track;
		}
	}

	AddDirtyFlags(EDopeSheetFlags::RebuildTracks);
	ForceUpdate();
}

TSet<FName> FDopeSheetController::GetAllCollectionNames() const
{
	TSet<FName> Names;
	if (TracksPtr)
	{
		for (UDopeSheetTrackBase* Track : *TracksPtr)
		{
			if (Track && Track->CollectionName != NAME_None)
			{
				Names.Add(Track->CollectionName);
			}
		}
	}
	return Names;
}

bool FDopeSheetController::IsCollectionVisible(FName InCollectionName) const
{
	if (InCollectionName == NAME_None) { return true; }
	const bool* bVisible = CollectionVisibility.Find(InCollectionName);
	return bVisible == nullptr || *bVisible;
}

void FDopeSheetController::MoveTracksToCollection(const TArray<UDopeSheetTrackBase*>& Tracks, FName NewCollection)
{
	// Register the collection as visible before modifying tracks so SetTracksSource
	// won't overwrite it with a default later.
	if (NewCollection != NAME_None && !CollectionVisibility.Contains(NewCollection))
	{
		CollectionVisibility.Add(NewCollection, true);
	}

	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "MoveToCollection_Transaction", "Move Tracks to Collection"), !GIsTransacting);
	for (UDopeSheetTrackBase* Track : Tracks)
	{
		if (Track)
		{
			Track->CollectionName = NewCollection;
			Track->Modify();
		}
	}

	if (TracksPtr)
	{
		SetTracksSource(*TracksPtr, TracksOwner);
	}
}

void FDopeSheetController::SetCollectionVisibility(FName InCollectionName, bool bVisible)
{
	CollectionVisibility.FindOrAdd(InCollectionName) = bVisible;

	if (!TracksPtr) { return; }

	// Rebuild root/visible tracks respecting new visibility
	RootTracks.Reset();
	VisibleTracks.Reset();
	ClearSelection();

	TSharedPtr<FDopeSheetController> ThisController = SharedThis(this);
	for (UDopeSheetTrackBase* Track : *TracksPtr)
	{
		if (!Track) { continue; }
		if (!IsCollectionVisible(Track->CollectionName)) { continue; }

		FDopeSheetTrackViewModelRef NewTrackModel = MakeShared<FDopeSheetTrackViewModel>(Track, ThisController);
		RootTracks.Add(NewTrackModel);
		VisibleTracks.Add(Track);

		for (UDopeSheetTrackBase* SubTrack : Track->SubTracks)
		{
			FDopeSheetTrackViewModelRef NewSubTrackModel = MakeShared<FDopeSheetTrackViewModel>(SubTrack, ThisController);
			NewTrackModel->Children.Add(NewSubTrackModel);
			NewSubTrackModel->ParentPtr = Track;
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
		
		int ParentIndex = VisibleTracks.IndexOfByKey(ExpandedTrackModel->ObjPtr);
		for (auto Child : ExpandedTrackModel->Children)
		{
			if (bExpanded)
			{
				VisibleTracks.Insert(Child->ObjPtr, ++ParentIndex);
			}
			else if (VisibleTracks.Contains(Child->ObjPtr))
			{
				VisibleTracks.Remove(Child->ObjPtr);
			}
		}
	}

	AddDirtyFlags(EDopeSheetFlags::VLayoutChanged);
	ForceUpdate();
}


void FDopeSheetController::AddViewSection(FDopeSheetViewSection NewSection)
{
	TotalNumberOfFrames += NewSection.NumOfFrames;
	ViewSections.Add(NewSection);
}

void FDopeSheetController::Reset(bool bMarkDirty)
{
	int DefaultNumFrames = 60;
	ViewSections.Reset(1);
	TotalNumberOfFrames = 0;

	if (bMarkDirty)
	{
		AddDirtyFlags(EDopeSheetFlags::All);
	}
}

void FDopeSheetController::OnKeyUp(const FGeometry& Geometry, const FKeyEvent& InKeyEvent)
{
	bShiftKeyDown = InKeyEvent.IsShiftDown();
}

void FDopeSheetController::OnKeyDown(const FGeometry& Geometry, const FKeyEvent& InKeyEvent)
{
	bShiftKeyDown = InKeyEvent.IsShiftDown();
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
		bIsPanning        = true;
		const float Delta = (InMouseEvent.GetCursorDelta().X / InGeometry.GetAbsoluteSize().X) * ViewTimeLength;
		SetViewRange(ViewStartTime - Delta, ViewEndTime - Delta);
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
			const bool bWasPanning = bIsPanning;
			bIsPanning     = false;
			bIsRMB_Pressed = false;
			InWidget->SetCursor(EMouseCursor::Default);

			if (!bWasPanning)
			{
				CreateContextMenu(MouseEvent, InWidget);
			}

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
		bIsPanning     = false;
		InWidget->SetCursor(EMouseCursor::GrabHandClosed);
		Reply = FReply::Handled().CaptureMouse(InWidget);
	}

	return Reply;
}

void FDopeSheetController::KeySelection()
{
	if (HasSelectionFlags(EDopeSheetSelectionFlags::RangeSelected))
	{
		for (int i = SelectionStartRow; i <= SelectionEndRow; i++)
		{
			if (VisibleTracks.IsValidIndex(i))
			{
				UDopeSheetTrackBase* SelectedTrack = VisibleTracks[i];
				if (SelectedTrack)
				{
					SelectedTrack->KeyTimeRange(SelectionStartTime, SelectionEndTime);
					SelectedTrack->OnTrackPropertiesChanged.Broadcast();


					AddDirtyFlags(EDopeSheetFlags::RebuildTracks);
					ForceUpdate();
				}
			}
		}

		ClearSelection();
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


	Selection_Start = MouseLocation;
	Selection_End   = Selection_Start;

	SelectionStartRow = -1;
	SelectionEndRow   = -1;

	SelectionFlags &= ~EDopeSheetSelectionFlags::RangeSelected;
}

void FDopeSheetController::DragSelection(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FVector2f MouseLocationAbsolute = InMouseEvent.GetScreenSpacePosition();
	FVector2f MouseLocation         = InGeometry.AbsoluteToLocal(MouseLocationAbsolute);
	Selection_End        =  MouseLocation;

	FVector2D A = FVector2D(TimeToXOffset(Selection_Start.X, InGeometry), Selection_Start.Y);
	FVector2D B = FVector2D(TimeToXOffset(Selection_End.X, InGeometry), Selection_End.Y);

	double StartX = ConvertXCoordToTime(Selection_Start.X, InGeometry);
	double EndX   = ConvertXCoordToTime(Selection_End.X, InGeometry);

	if (InMouseEvent.IsShiftDown() && !CachedSectionCellWidths.IsEmpty())
	{
		for (FDopeSheetViewSection ViewSection : ViewSections)
		{
			StartX *= ViewSection.NumOfFrames;
			EndX *= ViewSection.NumOfFrames;
		}

	}

	SelectionStartTime = StartX;
	SelectionEndTime   = EndX;

	SetPlayHeadTime(SelectionEndTime, true);

	if ( CachedTrackHeights.IsEmpty())
	{
		return;
	}

	FVector2D TopLeft(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
	FVector2D Size(FMath::Abs(A.X - B.X), FMath::Abs(A.Y - B.Y));
	FVector2D BotRight = TopLeft + Size;

	float CurrentY = 0.f;

	// Find which rows the selection spans
	for (int i = 0; i < CachedTrackHeights.Num(); ++i)
	{
		float NextY = CurrentY + CachedTrackHeights[i];
		if (TopLeft.Y < NextY)
		{
			SelectionStartRow = i;
			break;
		}
		CurrentY = NextY;
	}

	CurrentY = 0.f;
	for (int i = 0; i < CachedTrackHeights.Num(); ++i)
	{
		float NextY = CurrentY + CachedTrackHeights[i];

		if (BotRight.Y <= NextY)
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

	Selection_Start = FVector2f(0, 0);
	Selection_End = FVector2f(0, 0);

	SelectionStartRow   = -1;
	SelectionEndRow     = -1;
	SelectionStartFrame = -1;
	SelectionEndFrame   = -1;

	SelectionStartTime = -1;
	SelectionEndTime   = -1;


	SelectedSections.Empty();
}

void FDopeSheetController::SelectSection(TSharedPtr<FDopeSheetSectionViewModel>& SelectedSection, bool bIsAdditive)
{
	if (!bIsAdditive)
	{
		SelectedSections.Reset();
	}
	
	SelectedSections.Add(SelectedSection);
	OnSectionSelected.Broadcast(SelectedSection->Section);
}

void FDopeSheetController::DeleteSelection()
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "DeleteDopeSheetSelection_Transaction",
	                                               "Delete Selection"));

	for (auto SectionModel: SelectedSections)
	{
		UDopeSheetTrackBase* Track =  SectionModel->TrackModelPtr->ObjPtr;
		
		SectionModel->Section->MarkAsGarbage();
		Track->Sections.Remove(SectionModel->Section);
		Track->Modify();
		Track->OnTrackPropertiesChanged.Broadcast();
		
	}
	//
	// ObjPtr->OnTrackPropertiesChanged.Broadcast();
	//
	//
	// if (ObjPtr->Sections.IsValidIndex(SectionIndex))
	// {
	// 	if (UDopeSheetTrackSection* Section = ObjPtr->Sections[SectionIndex])
	// 	{
	// 		Section->MarkAsGarbage();
	// 		ObjPtr->Modify(true);
	// 		ObjPtr->Sections.RemoveAt(SectionIndex);
	// 	}
	// }	
}

void FDopeSheetController::ProcessBoxSelection()
{
}


FPaintGeometry FDopeSheetController::GetAdjustedSelectionGeometry(const FGeometry& InGeometry) const
{
	// FVector2D A = FVector2D(TimeToXOffset(Selection_Start.X, InGeometry), Selection_Start.Y);
	// FVector2D B = FVector2D(TimeToXOffset(Selection_End.X, InGeometry), Selection_End.Y);
	
	
	FVector2f A = FVector2f(Selection_Start.X, Selection_Start.Y);
	FVector2f B = FVector2f(Selection_End.X, Selection_End.Y);

	
	double StartX = ConvertXCoordToTime(Selection_Start.X, InGeometry);
	double EndX   = ConvertXCoordToTime(Selection_End.X, InGeometry);

	// if (bShiftKeyDown && !CachedSectionCellWidths.IsEmpty())
	// {
	// 	for (FDopeSheetViewSection ViewSection : ViewSections)
	// 	{
	// 		StartX *= ViewSection.NumOfFrames;
	// 		EndX *= ViewSection.NumOfFrames;
	// 	}
	//
	// }
	

	FVector2f TopLeft(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y));
	FVector2f Size(FMath::Abs(A.X - B.X), FMath::Abs(A.Y - B.Y));

	// return InGeometry.ToPaintGeometry(Size, FSlateLayoutTransform(TopLeft));
	
	const bool bBoxSelect = HasSelectionFlags(EDopeSheetSelectionFlags::BoxSelect);
	if (bBoxSelect || CachedTrackHeights.IsEmpty())
	{
		return InGeometry.ToPaintGeometry(Size, FSlateLayoutTransform(TopLeft));
	}

	float OffsetX       = TopLeft.X;
	float OffsetY       = TopLeft.Y;
	float SnappedHeight = Size.Y;

	if (SelectionStartRow >= 0 && SelectionEndRow >= 0 && SelectionStartRow <= SelectionEndRow)
	{
		OffsetY = 0.f;
		for (int i = 0; i < SelectionStartRow; ++i)
		{
			if (CachedTrackHeights.IsValidIndex(i))
			{
				OffsetY += CachedTrackHeights[i];
			}
		}

		SnappedHeight = 0.f;
		for (int i = SelectionStartRow; i <= SelectionEndRow; ++i)
		{
			SnappedHeight += CachedTrackHeights[i];
		}
	}
	return InGeometry.ToPaintGeometry(FVector2f(Size.X, SnappedHeight), FSlateLayoutTransform(FVector2f(TopLeft.X, OffsetY)));
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
			FString::Printf(TEXT("[%.2f-%.2f]"),
			                SelectionStartTime,
			                SelectionEndTime),
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

	ViewTimeLength = NewEndTime - NewStartTime;

	AddDirtyFlags(EDopeSheetFlags::HViewChanged);
	ForceUpdate();
}

void FDopeSheetController::FrameViewRange()
{
	const float EndTime = ViewSections.IsEmpty() ? 1 : ViewSections.Last().EndTime;
	SetViewRange(0.f, EndTime);
}

double FDopeSheetController::LocalXCoordToTime(double X) const
{
	return ((X / CachedGeometry.Size.X) * ViewTimeLength) + ViewStartTime;
}

double FDopeSheetController::AbsoluteXCoordToTime(double X) const
{
	return (((X - CachedGeometry.GetAbsolutePosition().X) / CachedGeometry.GetLocalSize().X) * ViewTimeLength) + ViewStartTime;
}

double FDopeSheetController::TimeToXOffset(const double InTime, const FGeometry& AllottedGeometry) const
{
	return AllottedGeometry.GetLocalSize().X * ((InTime - ViewStartTime) / ViewTimeLength);
}

double FDopeSheetController::ConvertXCoordToTime(const double XCoord, const FGeometry& AllottedGeometry) const
{
	return ((XCoord/AllottedGeometry.GetLocalSize().X) * ViewTimeLength) + ViewStartTime;
}


void FDopeSheetController::UpdateViewForGeometry(const FGeometry& AllottedGeometry)
{
	//Prevent Render thread from using stale cache

	if (ViewSections.IsEmpty())
	{
		return;
	}
	
	CachedGeometry = AllottedGeometry;


	//Cachce Frame Widths for each ViewSection
	CachedSectionCellWidths.Reset(ViewSections.Num());
	for (FDopeSheetViewSection ViewSection : ViewSections)
	{
		const float SectionDuration = ViewSection.EndTime - ViewSection.StartTime;
		const float SectionFraction = SectionDuration / ViewTimeLength;
		const float AllotedWidth    = (CachedGeometry.Size.X * SectionFraction);
		const float FrameWidth      = AllotedWidth / ViewSection.NumOfFrames;

		CachedSectionCellWidths.Add(FrameWidth);
	}

	//
	FDopeSheetViewSection FirstCellSection = ViewSections[0];
	FDopeSheetViewSection LastCellSection  = ViewSections.Last();

	const double ViewLeftBound  = TimeToXOffset(FirstCellSection.StartTime, CachedGeometry);
	const double ViewRightBound = TimeToXOffset(LastCellSection.EndTime, CachedGeometry);

	EditableRect = FSlateRect(ViewLeftBound, TimeSliderHeight.Get(), ViewRightBound,
	                            CachedGeometry.Size.Y - TimeSliderHeight.Get());
}

bool FDopeSheetController::CanDraw()
{
	if (ViewSections.Num() != CachedSectionCellWidths.Num()) { return false; }

	if (EnumHasAnyFlags(DirtyFlags, EDopeSheetFlags::Stale))
	{
		ForceUpdate(); //@todo: ~tim probably should be done in a tick function
		return false;
	}
	return true;
}

void FDopeSheetController::AddTrackToSelected(UClass* Class)
{
	if (TracksOwner && TracksPtr)
	{
		auto NewTrack = NewObject<UDopeSheetTrackBase>(TracksOwner, Class);
		TracksPtr->Add(NewTrack);

		TSharedPtr<FDopeSheetController> ThisController = SharedThis(this);
		FDopeSheetTrackViewModelRef      NewTrackModel  = MakeShared<FDopeSheetTrackViewModel>(NewTrack, ThisController);
		RootTracks.Add(NewTrackModel);
		VisibleTracks.Add(NewTrack);

		for (auto SubTrack : NewTrack->SubTracks)
		{
			FDopeSheetTrackViewModelRef NewSubTrackModel = MakeShared<FDopeSheetTrackViewModel>(
				SubTrack, ThisController);
			NewTrackModel->Children.Add(NewSubTrackModel);
		}
	}
	
	
	AddDirtyFlags(EDopeSheetFlags::RebuildTracks);
	ForceUpdate();
}
void FDopeSheetController::RemoveTrack( FDopeSheetTrackViewModelRef TrackModelRef)
{
	if (TrackModelRef->ParentPtr->SubTracks.Remove(TrackModelRef->ObjPtr))
	{
		TrackModelRef->ParentPtr->Modify();
	}
	
	if (TracksPtr->Remove(TrackModelRef->ObjPtr))
	{
		TracksOwner->Modify();
	}

	
	AddDirtyFlags(EDopeSheetFlags::RebuildTracks);
	ForceUpdate();
}

void FDopeSheetController::AddSubTrackToSelected(UClass* Class, FDopeSheetTrackViewModelRef TrackModelRef)
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "AddeDopeSheetTrack_Transaction", "Add DopeSheet Track"), !GIsTransacting);
	if (TrackModelRef->ObjPtr)
	{
		auto NewTrack = NewObject<UDopeSheetTrackBase>(TrackModelRef->ObjPtr, Class);
		TrackModelRef->ObjPtr->SubTracks.Add(NewTrack);
		TrackModelRef->ObjPtr->Modify();

		TSharedPtr<FDopeSheetController> ThisController = SharedThis(this);
		FDopeSheetTrackViewModelRef      NewTrackModel  = MakeShared<FDopeSheetTrackViewModel>(NewTrack, ThisController);
		TrackModelRef->Children.Add(NewTrackModel);
		
		for (auto SubTrack : NewTrack->SubTracks)
		{
			FDopeSheetTrackViewModelRef NewSubTrackModel = MakeShared<FDopeSheetTrackViewModel>(
				SubTrack, ThisController);
			NewTrackModel->Children.Add(NewSubTrackModel);
			NewTrackModel->ParentPtr = TrackModelRef->ObjPtr;
		}
	}

	AddDirtyFlags(EDopeSheetFlags::RebuildTracks);
	ForceUpdate();
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
