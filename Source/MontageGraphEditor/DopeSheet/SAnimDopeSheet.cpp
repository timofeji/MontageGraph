// Copyright Drop Games Inc.


#include "SAnimDopeSheet.h"

#include "DopeSheetController.h"
#include "EditorFontGlyphs.h"
#include "EditorStyleSet.h"
#include "EditorWidgetsModule.h"
#include "ITransportControl.h"
#include "MontageGraphEditorStyle.h"
#include "SDopeSheetOutliner.h"
#include "SlateOptMacros.h"
#include "SDopeSheetTimelineSlider.h"
#include "Tracks/DopeSheetTrackBase.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Fonts/FontMeasure.h"

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
		///Edit Commands
		UI_COMMAND(KeySelection, "Key Selection", "Key Selection", EUserInterfaceActionType::None,
		           FInputChord( EKeys::S));
		
		UI_COMMAND(DeleteSelection, "Delete Selection", "Delete Selection", EUserInterfaceActionType::None,
			FInputChord( EKeys::Delete));
		
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
	
	TSharedPtr<FUICommandInfo> DeleteSelection;
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
	
	for (int i = 0; i < Controller->ViewSections.Num(); i++)
	{
		FDopeSheetViewSection ViewSection = Controller->ViewSections[i];
		const float           CellWidth   = Controller->CachedSectionCellWidths[i];
		const int             NumOfFrames = ViewSection.NumOfFrames;

		// Determine which frame column the playhead sits in for this section
		int32 ActiveFrameInSection = -1;
		{
			const float PlayHeadTime    = Controller->GetPlayHeadTime();
			const float SectionDuration = ViewSection.EndTime - ViewSection.StartTime;
			if (SectionDuration > KINDA_SMALL_NUMBER
				&& PlayHeadTime >= ViewSection.StartTime
				&& PlayHeadTime <= ViewSection.EndTime)
			{
				const float T    = (PlayHeadTime - ViewSection.StartTime) / SectionDuration;
				ActiveFrameInSection = FMath::Clamp(FMath::FloorToInt(T * NumOfFrames), 0, NumOfFrames - 1);
			}
		}

		// float AccumulatedYOffset = Controller->TimeSliderHeight.Get();
		float AccumulatedYOffset = 0.f;
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
				const bool         bIsActive = (FrameIndex == ActiveFrameInSection);
				const FSlateBrush* Brush     = bIsActive ? CellBrush_Hover : CellBrush;
				const FLinearColor CellColor = bIsActive
					? FLinearColor(0.85f, 0.85f, 0.85f, 0.5f)
					: ((FrameIndex % 2 == 0) ? FLinearColor::White : FLinearColor(0.85f, 0.85f, 0.85f, 1.f));

				//////////////////////////////////////////////////////////////////////////
				// Draw frame cells
				FSlateDrawElement::MakeBox
				(
					OutDrawElements,
					LayerId + 1,
					AllottedGeometry.ToPaintGeometry(
						FVector2D(AccumulatedXOffset + (CellWidth * FrameIndex), AccumulatedYOffset),
						FVector2D(CellWidth, FrameHeight)),
					Brush,
					ESlateDrawEffect::None,
					CellColor
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
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Clipping(EWidgetClipping::Inherit)
		[
			SAssignNew(TrackVerticalBox, SVerticalBox)
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
	// Draw PlayHead & PlayLine
	//////////////////////////////////////////////////////////////////////////
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
	
	//////////////////////////////////////////////////////////////////////////
	// Draw Void Geometry
	//////////////////////////////////////////////////////////////////////////
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

	//////////////////////////////////////////////////////////////////////////
	// Draw Stats Overlay — floats at the bottom of the playhead line
	//////////////////////////////////////////////////////////////////////////
	if (PlayHeadTime >= Controller->ViewStartTime
		&& PlayHeadTime <= Controller->ViewEndTime
		&& !Controller->ViewSections.IsEmpty())
	{
		// Reuse the same floor-based frame calculation as the cell renderer
		int32 CurrentFrame = 0;
		{
			int32 Accumulated = 0;
			for (const FDopeSheetViewSection& Section : Controller->ViewSections)
			{
				const float Dur = Section.EndTime - Section.StartTime;
				if (Dur > KINDA_SMALL_NUMBER
					&& (PlayHeadTime <= Section.EndTime || &Section == &Controller->ViewSections.Last()))
				{
					const float T = FMath::Clamp((PlayHeadTime - Section.StartTime) / Dur, 0.f, 1.f);
					CurrentFrame  = Accumulated + FMath::Clamp(FMath::FloorToInt(T * Section.NumOfFrames), 0, Section.NumOfFrames - 1);
					break;
				}
				Accumulated += Section.NumOfFrames;
			}
		}

		const int32   TotalFrames   = Controller->TotalNumberOfFrames;
		const float   TotalDuration = Controller->ViewSections.Last().EndTime;
		const FString StatsText     = FString::Printf(
			TEXT("%d/%d  (%.2fs/%.2fs)"),
			CurrentFrame, TotalFrames, PlayHeadTime , TotalDuration);

		static const FSlateFontInfo StatsFont  = FCoreStyle::GetDefaultFontStyle("Regular", 9);
		static const FLinearColor   StatsColor = FLinearColor(1.f, 1.f, 1.f, 0.6f);

		TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FVector2D TextMeasure = FontMeasure->Measure(StatsText, StatsFont);

		// Center on the playhead line, then clamp so the label never clips
		// outside the editable rect on either side.
		const float LineXPos = Controller->TimeToXOffset(PlayHeadTime, AllottedGeometry);
		const float TextX    = FMath::Clamp(
			LineXPos - TextMeasure.X * 0.5f,
			ViewRect.Left,
			ViewRect.Right - TextMeasure.X);
		const float TextY    = AllottedGeometry.Size.Y - TextMeasure.Y - 6.f;

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(FVector2D(TextX, TextY), TextMeasure),
			StatsText,
			StatsFont,
			ESlateDrawEffect::None,
			StatsColor
		);
	}

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

FReply SDopeSheet::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	Controller->OnKeyDown(MyGeometry, InKeyEvent);
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SDopeSheet::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	Controller->OnKeyUp(MyGeometry, InKeyEvent);
	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
}

void SDopeSheet::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	Controller->UpdateViewForGeometry(AllottedGeometry);
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
		
		
	FEditorWidgetsModule& EditorWidgetsModule = FModuleManager::Get().LoadModuleChecked<FEditorWidgetsModule>("EditorWidgets");
	// Transport control args
	{
		FTransportControlArgs TransportControlArgs;
		TransportControlArgs.OnGetPlaybackMode.BindLambda([&]() -> EPlaybackMode::Type { return Controller->PlaybackMode; });
		// TransportControlArgs.OnBackwardEnd.BindSP(this, &SNiagaraBakerWidget::OnTransportBackwardEnd);
		// TransportControlArgs.OnBackwardStep.BindSP(this, &SNiagaraBakerWidget::OnTransportBackwardStep);
		// TransportControlArgs.OnBackwardPlay.BindSP(this, &SNiagaraBakerWidget::OnTransportBackwardPlay);
		// TransportControlArgs.OnForwardPlay.BindSP(this, &SNiagaraBakerWidget::OnTransportForwardPlay);
		// TransportControlArgs.OnForwardStep.BindSP(this, &SNiagaraBakerWidget::OnTransportForwardStep);
		// TransportControlArgs.OnForwardEnd.BindSP(this, &SNiagaraBakerWidget::OnTransportForwardEnd);
		// TransportControlArgs.OnToggleLooping.BindSP(this, &SNiagaraBakerWidget::OnTransportToggleLooping);
		// TransportControlArgs.OnGetLooping.BindSP(ViewModel, &FNiagaraBakerViewModel::IsPlaybackLooping);
		//
		TransportControlArgs.WidgetsToCreate.Add(FTransportControlWidget(ETransportControlWidgetType::BackwardEnd));
		TransportControlArgs.WidgetsToCreate.Add(FTransportControlWidget(ETransportControlWidgetType::BackwardStep));
		TransportControlArgs.WidgetsToCreate.Add(FTransportControlWidget(ETransportControlWidgetType::BackwardPlay));
		TransportControlArgs.WidgetsToCreate.Add(FTransportControlWidget(ETransportControlWidgetType::ForwardPlay));
		TransportControlArgs.WidgetsToCreate.Add(FTransportControlWidget(ETransportControlWidgetType::ForwardStep));
		TransportControlArgs.WidgetsToCreate.Add(FTransportControlWidget(ETransportControlWidgetType::ForwardEnd));
		TransportControlArgs.WidgetsToCreate.Add(FTransportControlWidget(ETransportControlWidgetType::Loop));

		TransportControlArgs.bAreButtonsFocusable = false;

		TransportControls = EditorWidgetsModule.CreateTransportControl(TransportControlArgs);
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.FillHeight(1.f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)
			.Style(FAppStyle::Get(), "DetailsView.Splitter")
			+ SSplitter::Slot()
			.MinSize(15.f)
			.Value(0.175f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoHeight()
				.MaxHeight(Controller->TimeSliderHeight)
				.MinHeight(Controller->TimeSliderHeight)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(6)
					.FillWidth(1.f)
					[
						SAssignNew(OutlinerFilterBox, SSearchBox)
						.OnTextChanged(this, &SAnimDopeSheet::OnOutlinerFilterTextChanged)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(CollectionsButton, SComboButton)
						.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButton")
						.HasDownArrow(true)
						.OnGetMenuContent(this, &SAnimDopeSheet::MakeCollectionsMenuContent)
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CollectionsButton", "Collections"))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
						]
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				.Padding(0)
				[
					SAssignNew(OutlinerWidget, SDopeSheetOutliner, Controller)
					.ExternalScrollbar(ScrollBar.ToSharedPtr())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0)
				[
					TransportControls.ToSharedRef()
				]
			]
			+ SSplitter::Slot()
			.Value(0.825f)
			[
				// SNew(SBox)

				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				[

					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.Padding(0)
					.MaxHeight(Controller->TimeSliderHeight)
					.MinHeight(Controller->TimeSliderHeight)
					.AutoHeight()
					[
						SNew(SDopeSheetTimelineSlider, InController)
					]
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					.FillHeight(1.f)
					.Padding(0)
					[
						SAssignNew(TimelineScrollBox, SScrollBox)
						.Orientation(Orient_Vertical)
						.ScrollBarVisibility(EVisibility::Collapsed)
						.OnUserScrolled_Lambda([this](float PixelOffset)
						{
							if (OutlinerWidget.IsValid())
							{
								OutlinerWidget->SetScrollOffset(static_cast<double>(PixelOffset) / 32.0);
							}
						})
						+ SScrollBox::Slot()
						[
							SAssignNew(DopeSheet, SDopeSheet, Controller)
						]
					]
				]
			]

		]
	];
}
	// + SVerticalBox::Slot()
				// .HAlign(HAlign_Fill)
				// .VAlign(VAlign_Bottom)
				// [
				// 	SNew(SButton)
				// 	.ButtonStyle(FAppStyle::Get(), "FlatButton.Danger")
				// 	.ToolTipText(NSLOCTEXT("DopeSheetOutliner", "RemoveTracksButtonToolTip", "Remove Tracks"))
				// 	.IsEnabled(OutlinerWidget.Get(), &SDopeSheetOutliner::IsDeleteEnabled)
				// 	.OnClicked_Lambda([this] {
				// 		OutlinerWidget->OnRemoveTrackClicked();
				// 		return FReply::Handled();
				// 	})
				// 	[
				// 		SNew(SHorizontalBox)
				// 		+ SHorizontalBox::Slot()
				// 		.AutoWidth()
				// 		.VAlign(VAlign_Center)
				// 		.Padding(2.f, 0.f)
				// 		[
				// 			SNew(SImage)
				// 			.Image(FAppStyle::GetBrush("Icons.Plus"))
				// 			.ColorAndOpacity(FSlateColor::UseForeground())
				// 		]
				// 		+ SHorizontalBox::Slot()
				// 		.AutoWidth()
				// 		.VAlign(VAlign_Center)
				// 		.Padding(2.f, 0.f)
				// 		[
				// 			SNew(STextBlock)
				// 			.Text(FEditorFontGlyphs::Times)
				// 			.TextStyle(FAppStyle::Get(), "FlatButton.DefaultTextStyle")
				// 			.Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
				// 		]
				// 	]
				// ]

void SAnimDopeSheet::OnOutlinerFilterTextChanged(const FText& Text)
{
	OutlinerWidget->SetFilterText(Text);
};

void SAnimDopeSheet::RequestRefresh()
{
	// Invalidate(EInvalidateWidgetReason::Layout);
}

void SAnimDopeSheet::SetTracksSource(TArray<UDopeSheetTrackBase*>& Array)
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
	
	ActionList.MapAction(Commands.DeleteSelection,
		FExecuteAction::CreateRaw(this, &SAnimDopeSheet::DeleteSelection),
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

void SAnimDopeSheet::DeleteSelection()
{
	Controller->DeleteSelection();
}

void SAnimDopeSheet::KeySelection()
{
	Controller->KeySelection();
}

bool SAnimDopeSheet::CanKeySelection()
{
	return Controller->HasSelectionFlags(EDopeSheetSelectionFlags::RangeSelected);
}

TSharedRef<SWidget> SAnimDopeSheet::MakeCollectionsMenuContent()
{
	const bool bShouldCloseAfterSelection = false;
	FMenuBuilder MenuBuilder(bShouldCloseAfterSelection, nullptr);

	TSet<FName> CollectionNames = Controller->GetAllCollectionNames();

	if (CollectionNames.IsEmpty())
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("NoCollections", "(no collections)"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction()
		);
	}
	else
	{
		MenuBuilder.BeginSection("Collections", LOCTEXT("CollectionsSectionHeader", "Collections"));
		for (const FName& Name : CollectionNames)
		{
			MenuBuilder.AddMenuEntry(
				FText::FromName(Name),
				FText::GetEmpty(),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([this, Name]()
					{
						const bool bCurrentlyVisible = Controller->IsCollectionVisible(Name);
						Controller->SetCollectionVisibility(Name, !bCurrentlyVisible);
					}),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda([this, Name]()
					{
						return Controller->IsCollectionVisible(Name);
					})
				),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

void SAnimDopeSheet::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (OutlinerWidget.IsValid() && TimelineScrollBox.IsValid())
	{
		const float TargetPixels = static_cast<float>(OutlinerWidget->GetScrollOffset()) * 32.f;
		if (FMath::Abs(TargetPixels - TimelineScrollBox->GetScrollOffset()) > 0.5f)
		{
			TimelineScrollBox->SetScrollOffset(TargetPixels);
		}
	}
}

#undef LOCTEXT_NAMESPACE
