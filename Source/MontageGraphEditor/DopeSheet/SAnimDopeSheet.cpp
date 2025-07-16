// Copyright Drop Games Inc.


#include "SAnimDopeSheet.h"

#include "DopeSheetController.h"
#include "MontageGraphEditorStyle.h"
#include "SDopeSheetOutliner.h"
#include "SlateOptMacros.h"
#include "SDopeSheetTimelineSlider.h"
#include "Tracks/DopeSheetTrackBase.h"
#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "AnimationDopeSheet"

class FDopeSheetCommands : public TCommands<FDopeSheetCommands>
{
public:
	FDopeSheetCommands()
		: TCommands<FDopeSheetCommands>(
			TEXT("AutomationWindow"),
			NSLOCTEXT("Contexts", "AutomationWindow", "Automation Window"),
			NAME_None, FMontageGraphEditorStyle::Get().GetStyleSetName()
		)
	{
	}

	virtual void RegisterCommands() override
	{
		///////////////////////////////////////////////////////////////
		///Anim Commands
		UI_COMMAND(KeySelection, "Key Selection", "Key Selection", EUserInterfaceActionType::None,
		           FInputChord( EKeys::S));
		
		///////////////////////////////////////////////////////////////
		///Timeline Commands
		UI_COMMAND(TogglePlayback, "Play Timeline", "Play Timeline", EUserInterfaceActionType::Button,
		           FInputChord(EKeys::SpaceBar));

		///////////////////////////////////////////////////////////////
		///View Commands
		UI_COMMAND(ResetView, "Reset View", "Reset View", EUserInterfaceActionType::None, FInputChord(EKeys::F));

		///////////////////////////////////////////////////////////////
		///Track Commands
		UI_COMMAND(FindTrack, "Find Track", "Find Track", EUserInterfaceActionType::Button,
		           FInputChord(EModifierKey::Control, EKeys::F));
	}

public:
	TSharedPtr<FUICommandInfo> TogglePlayback;
	TSharedPtr<FUICommandInfo> ResetView;
	TSharedPtr<FUICommandInfo> FindTrack;
	
	TSharedPtr<FUICommandInfo> KeySelection;
};


FDopeSheetFrameCellRenderer::FDopeSheetFrameCellRenderer()
{
	CellBrush       = FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.DopeSheet.Cell");
	CellBrush_Hover = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.DopeSheet.Cell.Hover");


	LineColor = FLinearColor::White;
}

inline void FDopeSheetFrameCellRenderer::Update(const TSharedPtr<FDopeSheetController>& Controller, const FGeometry& AllottedGeometry)
{

	FrameGeometries.Reset();
	// FrameGeometries.AddUninitialized(2 * NumOfFrames);
	
	// for (FFrameNumber FrameIndex : Controller->EditableView)
	// {
		// const float LineXPos = AccumulatedXOffset + FrameIndex * FrameWidth;
		// FrameGeometries.Add(FVector2f(LineXPos, 0));
		// FrameGeometries.Add(FVector2f(LineXPos, AllottedGeometry.Size.Y));
		//
		// AccumulatedXOffset += LineXPos;
	// }
}

inline void FDopeSheetFrameCellRenderer::Draw(FSlateWindowElementList&                OutDrawElements,
                                              int32                                   LayerId,
                                              const FGeometry&                        AllottedGeometry,
                                              const TSharedPtr<FDopeSheetController>& Controller) const
{

	
	FSlateRect ViewRect = Controller->EditableRect;
	float AccumulatedXOffset = ViewRect.Left;
	
	for (int i = 0; i < Controller->Sections.Num(); i++)
	{
		FDopeSheetViewSection ViewSection = Controller->Sections[i];
		const float           CellWidth   = Controller->CachedSectionCellWidths[i];
		const int             NumOfFrames = ViewSection.NumOfFrames;


		float AccumulatedYOffset = Controller->TimeSliderHeight.Get();
		for (int TrackIndex = 0; TrackIndex < Controller->VisibleTracks.Num(); TrackIndex++)
		{
			const UDopeSheetTrackBase* Track = Controller->VisibleTracks[TrackIndex];
			const float FrameHeight = Track->GetTrackHeight();
			
			if (!Track->bShouldDrawCells)
			{
				AccumulatedYOffset += FrameHeight;
				continue;
			}
			
			
			for (int FrameIndex = 0; FrameIndex < NumOfFrames; FrameIndex++)
			{
				FLinearColor CellColor = FLinearColor::White;

				//////////////////////////////////////////////////////////////////////////
				// Draw frame cells
				FSlateDrawElement::MakeBox
				(
					OutDrawElements,
					LayerId + 1,
					AllottedGeometry.ToPaintGeometry(
						FVector2D(AccumulatedXOffset + (CellWidth * FrameIndex), AccumulatedYOffset),
						FVector2D(CellWidth, FrameHeight)),
					CellBrush,
					ESlateDrawEffect::None,
					(FrameIndex % 2 == 0) ? CellColor : CellColor.Desaturate(0.2f)
				);
			}
			AccumulatedYOffset += FrameHeight;
		}
		AccumulatedXOffset += CellWidth * NumOfFrames;
	}
}


static const FLinearColor VoidBrushColor = FLinearColor(.1f, .1f, .1f, .25f);

void SDopeSheet::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController)
{

	VoidBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.TimelineSlider.Void");


	Controller = InController;
	Controller->OnUpdate.AddLambda([this](EDopeSheetFlags UpdatedFlags)
	{
		CellRenderer.Update(Controller, GetCachedGeometry());
		
		if (EnumHasAnyFlags(UpdatedFlags, EDopeSheetFlags::AllHorizontal))
		{
			Controller->UpdateViewForGeometry(GetCachedGeometry());
		}

		if (EnumHasAnyFlags(UpdatedFlags, EDopeSheetFlags::RebuildTracks))
		{
			RebuildTracks();
		}
	});

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.MaxHeight(Controller->TimeSliderHeight)
		[
			SNew(SDopeSheetTimelineSlider, InController)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SBox)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SAssignNew(TrackVerticalBox, SVerticalBox)
				]
			]
		]
	];


}

int32 SDopeSheet::OnPaint(const FPaintArgs&        Args, const FGeometry& AllottedGeometry,
                          const FSlateRect&        MyCullingRect,
                          FSlateWindowElementList& OutDrawElements, int32 LayerId,
                          const FWidgetStyle&      InWidgetStyle,
                          bool                     bParentEnabled) const
{
	
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
	                                   InWidgetStyle,
	                                   bParentEnabled);
	
	if (!Controller || !Controller->CanDraw())
	{
		return LayerId;
	}

	OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry.GetLayoutBoundingRect()));

	//////////////////////////////////////////////////////////////////////////
	// Draw PlayHeadTime line
	float PlayHeadTime = Controller->GetPlayHeadTime();
	if (PlayHeadTime >= Controller->ViewStartTime
		&& PlayHeadTime <= Controller->ViewEndTime)
	{
		TArray<FVector2f> LinePoints;
		LinePoints.AddUninitialized(2);

		const float LineXPos = Controller->TimeToXOffset(PlayHeadTime, AllottedGeometry);
		LinePoints[0]        = FVector2f(LineXPos, Controller->TimeSliderHeight.Get());
		LinePoints[1]        = FVector2f(LineXPos, AllottedGeometry.Size.Y);

		constexpr float Thickness    = 1.f;
		constexpr float DashLengthPx = 3.f;
		FSlateDrawElement::MakeDashedLines(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(),
			MoveTemp(LinePoints),
			ESlateDrawEffect::None,
			FLinearColor(255, 255, 255, 125),
			Thickness,
			DashLengthPx
		);
	}

	const FSlateRect ViewRect = Controller->EditableRect;
	constexpr float Thickness    = 1.f;
	constexpr float DashLengthPx = 3.f;

	TArray<FVector2f> LinePoints;
	{
		LinePoints.AddUninitialized(2);

		const float StartLineXPos = ViewRect.Left;
		LinePoints[0]             = FVector2f(StartLineXPos, 0);
		LinePoints[1]             = FVector2f(StartLineXPos, AllottedGeometry.Size.Y);

		FSlateDrawElement::MakeDashedLines(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(),
			MoveTemp(LinePoints),
			ESlateDrawEffect::None,
			FLinearColor(0, 255, 0, 125),
			Thickness,
			DashLengthPx
		);
	}
	{
		LinePoints.AddUninitialized(2);
		
		const float EndLineXPos = ViewRect.Right;
		LinePoints[0]           = FVector2f(EndLineXPos, 0);
		LinePoints[1]           = FVector2f(EndLineXPos, AllottedGeometry.Size.Y);

		FSlateDrawElement::MakeDashedLines(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(),
			MoveTemp(LinePoints),
			ESlateDrawEffect::None,
			FLinearColor(255, 0, 0, 125),
			Thickness,
			DashLengthPx
		);
	}

	{
		FPaintGeometry VoidGeometry =
			AllottedGeometry.ToPaintGeometry(FVector2D(0, 0.f + Controller->TimeSliderHeight.Get()),
			                                 FVector2D(ViewRect.Left, AllottedGeometry.Size.Y));

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId,
			VoidGeometry,
			VoidBrush,
			ESlateDrawEffect::None,
			VoidBrushColor
		);
	}
	{
		FPaintGeometry VoidGeometry =
			AllottedGeometry.ToPaintGeometry(FVector2D(ViewRect.Right, 0.f + Controller->TimeSliderHeight.Get()),
			                                 FVector2D(AllottedGeometry.Size.X - ViewRect.Right,
			                                           AllottedGeometry.Size.Y));

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId,
			VoidGeometry,
			VoidBrush,
			ESlateDrawEffect::None,
			VoidBrushColor
		);
	}


	//////////////////////////////////////////////////////////////////////////
	// Draw Track Cells
	//////////////////////////////////////////////////////////////////////////
	CellRenderer.Draw(OutDrawElements, LayerId, AllottedGeometry, Controller);

	//////////////////////////////////////////////////////////////////////////
	// Draw Selection Box
	//////////////////////////////////////////////////////////////////////////
	Controller->DrawSelection(OutDrawElements, LayerId + 2, AllottedGeometry);

	OutDrawElements.PopClip();
	return LayerId;
}

FReply SDopeSheet::OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Controller->OnMouseButtonDown(InGeometry, InMouseEvent, SharedThis(this));
}

FReply SDopeSheet::OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Controller->OnMouseButtonUp(InGeometry, InMouseEvent, SharedThis(this));
}

FReply SDopeSheet::OnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Controller->OnMouseMove(InGeometry, InMouseEvent);
}

FReply SDopeSheet::OnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& MouseEvent)
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	/// Resize Track Height
	// if (MouseEvent.IsAltDown() && MouseEvent.IsControlDown() && TrackUnderCursor)
	// {
	// 	FDopeSheetTrackViewModelRef HighlightedTrack = *TrackUnderCursor;
	// 	float                       HeightDelta      = FMath::CeilToInt(MouseEvent.GetWheelDelta());
	// 	HighlightedTrack->SetTrackHeight(HighlightedTrack->TrackHeight + HeightDelta);
	// }
	////////////////////////////////////////////////////////////////////////////////////////////////
	/// Scroll View Horizontally(Move Left/Right)
	 if (MouseEvent.IsShiftDown())
	{
		float Delta = MouseEvent.GetWheelDelta() * .1f;
		Controller->ScrollView(Delta);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	/// ZoomIn View (Narrows down the view range)
	else if (MouseEvent.IsControlDown())
	{
		FVector2f   MouseLocationAbsolute = MouseEvent.GetScreenSpacePosition();
		FVector2f   MouseLocation         = InGeometry.AbsoluteToLocal(MouseLocationAbsolute);
		const float Center                = Controller->ConvertXCoordToTime(MouseLocation.X, InGeometry);
		const float Delta                 = MouseEvent.GetWheelDelta() * -.1f;
		Controller->ZoomInView(Delta, Center);
	}

	return SCompoundWidget::OnMouseWheel(InGeometry, MouseEvent);
}


void SDopeSheet::RebuildTracks()
{
	if (Controller->RootTracks.IsEmpty())
	{
		return;
	}

	TrackVerticalBox->ClearChildren();
	for (auto TrackModel : Controller->RootTracks)
	{
		TrackVerticalBox->AddSlot()
		                .VAlign(VAlign_Top)
		                .AutoHeight()
		[
			TrackModel->MakeTrackTimelineWidget()
		];
		

		for (auto SubTrackModel : TrackModel->Children)
		{
			TrackVerticalBox->AddSlot()
			                .VAlign(VAlign_Top)
			                .AutoHeight()
			[
				SubTrackModel->MakeTrackTimelineWidget()
			];
		}
	}

}

/** Constructs this widget with InArgs */
void SAnimDopeSheet::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController)
{
	Controller = InController;

	CreateDopeSheetCommands();


	TSharedRef<SScrollBar> ScrollBar =
		SNew(SScrollBar)
		.Thickness(FVector2D(6.0f, 6.0f));

	ChildSlot
	[
		SNew(SSplitter)
		.Orientation(Orient_Horizontal)
		.Style(FAppStyle::Get(), "DetailsView.Splitter")
		+ SSplitter::Slot()
		.Value(0.175f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.MaxHeight(Controller->TimeSliderHeight)
			[
				SAssignNew(OutlinerFilterBox, SSearchBox)
				.OnTextChanged(this, &SAnimDopeSheet::OnOutlinerFilterTextChanged)
			]
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.f)
			.FillContentHeight(1.f)
			.Padding(0, 2)
			[
				SAssignNew(OutlinerWidget, SDopeSheetOutliner, Controller)
				.ExternalScrollbar(ScrollBar.ToSharedPtr())
			]
		]
		+ SSplitter::Slot()
		.Value(0.825f)
		[
			SNew(SBox)
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(DopeSheet, SDopeSheet, Controller)
			]
		]
	];
}


void SAnimDopeSheet::OnOutlinerFilterTextChanged(const FText& Text)
{
	OutlinerWidget->SetFilterText(Text);
};

void SAnimDopeSheet::RequestRefresh()
{
	// Invalidate(EInvalidateWidgetReason::Layout);
}

void SAnimDopeSheet::SetTracksSource(const TArray<UDopeSheetTrackBase*>& Array)
{
	Controller->SetTracksSource(Array);

	if (OutlinerWidget.IsValid())
	{
		OutlinerWidget->RequestTreeRefresh();
	}

	RequestRefresh();
}


void SAnimDopeSheet::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);

	// FSlateApplication::Get().ClearKeyboardFocus();
}

void SAnimDopeSheet::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);

	// FSlateApplication::Get().SetKeyboardFocus(SharedThis(this), EFocusCause::SetDirectly);
}

void SAnimDopeSheet::CreateDopeSheetCommands()
{
	FDopeSheetCommands::Register();

	check(!DopeSheetActions.IsValid());
	DopeSheetActions = MakeShareable(new FUICommandList);

	const FDopeSheetCommands& Commands   = FDopeSheetCommands::Get();
	FUICommandList&           ActionList = *DopeSheetActions;
	
	
	ActionList.MapAction(Commands.KeySelection,
	                     FExecuteAction::CreateRaw(this, &SAnimDopeSheet::KeySelection),
	                     FCanExecuteAction::CreateLambda([] { return true; })
	);


	ActionList.MapAction(Commands.TogglePlayback,
	                     FExecuteAction::CreateRaw(this, &SAnimDopeSheet::TogglePlayback),
	                     FCanExecuteAction::CreateLambda([] { return true; })
	);


	ActionList.MapAction(Commands.ResetView,
	                     FExecuteAction::CreateRaw(this, &SAnimDopeSheet::ResetView),
	                     FCanExecuteAction::CreateLambda([] { return true; })
	);


	ActionList.MapAction(Commands.FindTrack,
	                     FExecuteAction::CreateRaw(this, &SAnimDopeSheet::FindTrack),
	                     FCanExecuteAction::CreateLambda([] { return true; })
	);
}


FReply SAnimDopeSheet::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (DopeSheetActions.IsValid() && DopeSheetActions->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SAnimDopeSheet::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// if (DopeSheetActions.IsValid() && DopeSheetActions->ProcessCommandBindings(InKeyEvent))
	// {
	// 	return FReply::Handled();
	// }

	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
}


void SAnimDopeSheet::TogglePlayback()
{
	Controller->SetPlaybackState(!Controller->bIsPlayingAnimation);
};


void SAnimDopeSheet::ResetView() const
{
	Controller->FrameViewRange();
}

void SAnimDopeSheet::FindTrack()
{
	// FSlateApplication::Get().SetKeyboardFocus(OutlinerFilterBox, EFocusCause::SetDirectly);
	// FSlateApplication::Get().SetUserFocus(0, OutlinerFilterBox);
}



void SAnimDopeSheet::KeySelection()
{
	Controller->KeySelection();
}

bool SAnimDopeSheet::CanKeySelection()
{
	return Controller->HasSelectionFlags(EDopeSheetSelectionFlags::RangeSelected);
}

#undef LOCTEXT_NAMESPACE
