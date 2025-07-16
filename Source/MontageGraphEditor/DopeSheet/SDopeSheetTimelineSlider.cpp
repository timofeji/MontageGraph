// Copyright Drop Games Inc.


#include "SDopeSheetTimelineSlider.h"

#include "DopeSheetController.h"
#include "MontageGraphEditorStyle.h"
#include "SlateOptMacros.h"
#include "Fonts/FontMeasure.h"



TSharedPtr<SWidget> FDopeSheetTimelineSliderScrubOp::GetDefaultDecorator() const
{
	return WidgetBeingDragged.Pin();
}

void FDopeSheetTimelineSliderScrubOp::Construct()
{
	FDragDropOperation::Construct();

	CursorDecoratorWindow->SetOpacity(.25f);
}

void FDopeSheetTimelineSliderScrubOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	// if ( bDropWasHandled == false )
	// {
	// 	if (SectionBeingDragged.IsValid())
	// 	{
	// 		SectionBeingDragged.Pin()->OnDropCancelled(MouseEvent);
	// 	}
	// }
	//

	// if (OwningTrack.IsValid())
	// {
	// 	// OwningTrack.Pin()->OnRearrangeDrop();
	// }


	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}


void FDopeSheetTimelineSliderScrubOp::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	// FSlateRect BoundingRect = OwningTrack.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
	// FSlateRect SectionRect  = SectionBeingDragged.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
	//
	//
	// FVector2D MousePos = DragDropEvent.GetScreenSpacePosition();
	// FVector2D SectionDerotatowPos;
	// SectionDerotatowPos.X = FMath::Clamp((MousePos + Offset).X,
	//                                      BoundingRect.Left,
	//                                      BoundingRect.Right - SectionRect.GetSize().X);
	// SectionDerotatowPos.Y = StartingScreenPos.Y;
	// CursorDecoratorWindow->MoveWindowTo(SectionDerotatowPos);

	// OwningTrack.Pin()->PreviewRearrange(MousePos);
}

TSharedRef<FDopeSheetTimelineSliderScrubOp> FDopeSheetTimelineSliderScrubOp::New(
	TSharedPtr<FDopeSheetController> InController,
	TSharedRef<SWidget> InDraggedWidget, const FVector2D& CursorPosition, const FVector2D& ScreenPositionOfNode)
{
	TSharedRef<FDopeSheetTimelineSliderScrubOp> Operation = MakeShareable(new FDopeSheetTimelineSliderScrubOp);

	Operation->Controller         = InController;
	Operation->WidgetBeingDragged = InDraggedWidget;
	Operation->Offset             = ScreenPositionOfNode - CursorPosition;
	Operation->StartingScreenPos  = ScreenPositionOfNode;

	Operation->Construct();


	return Operation;
}


namespace FDopeSheetTimelineRenderer
{
	static FSlateFontInfo FontInfo = FAppStyle::GetFontStyle("Graph.VectorEditableTextBox");

	struct FFrameDisplaySettings
	{
		int32 MajorTickInterval = 5;
		int32 MinorTickInterval = 1;
		float MinTextSpacing    = 2.0f; // Minimum pixels between text labels

		FLinearColor StartTextColor   = FLinearColor(0.8f, 0.8f, 0.8f, 1.f);
		FLinearColor MajorTextColor   = FLinearColor(0.8f, 0.8f, 0.8f, 0.75f);
		FLinearColor DefaultTextColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.2f);
		
		
		FLinearColor TickColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.1f);
		FLinearColor TickColorMajor = FLinearColor(0.5f, 0.5f, 0.5f, 0.25f);


		const FVector2D TextSize = FVector2D(32.f);
		// = FSlateApplication::Get().GetRenderer()->
		//                                                     GetFontMeasureService()->Measure(TEXT("0"), FontInfo);
	};

	FFrameDisplaySettings Settings;

	static void RenderTimeline(FSlateWindowElementList&                OutDrawElements,
	                           int32                                   LayerId,
	                           const FGeometry&                        AllottedGeometry,
	                           const TSharedPtr<FDopeSheetController>& Controller)
	{
		const FSlateRect ViewRect = Controller->EditableRect;
		float      AccumulatedXOffset = ViewRect.Left;
		for (int i = 0; i < Controller->Sections.Num(); i++)
		{
			FDopeSheetViewSection ViewSection   = Controller->Sections[i];
			const float           PixelsPerCell      = Controller->CachedSectionCellWidths[i];
			const int             NumOfFrames   = ViewSection.NumOfFrames;
			
			TArray<FVector2f>     LinePoints;
			LinePoints.AddUninitialized(2);
			for (int FrameIndex = 0; FrameIndex < NumOfFrames; FrameIndex++)
			{
				const float OffsetX = AccumulatedXOffset  + (PixelsPerCell * FrameIndex);

				FLinearColor TextColor   = Settings.DefaultTextColor;
				float        TickYOffset = Settings.TextSize.Y + 5.f;
				
				FVector2f    Offset(OffsetX, TickYOffset);
				FVector2f    TickSize(1.0f, AllottedGeometry.Size.Y);

				LinePoints[0] = FVector2f(1.0f, 1.0f);
				LinePoints[1] = TickSize;

				const bool bAntiAliasLines = false;
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId + 2,
					AllottedGeometry.ToPaintGeometry(TickSize, FSlateLayoutTransform(Offset)),
					LinePoints,
					ESlateDrawEffect::None,
					Settings.TickColor,
					false
				);

				const FString FrameString   = FString::FromInt(FrameIndex);
				const bool    bFitsIntoCell = PixelsPerCell > ((2 * Settings.TextSize.X) + Settings.MinTextSpacing);

				if ((FrameIndex % Settings.MajorTickInterval == 0))
				{
					TextColor   = Settings.MajorTextColor;
					TickYOffset = 0;
				}
				else if (!bFitsIntoCell) continue;

				float TextXOffset = OffsetX;

				if (FrameIndex == 0)
				{
					// Left align for first frame
					TextXOffset += 2.0f;
				}
				else if (FrameIndex == NumOfFrames - 1)
				{
					// Right align for last frame
					TextXOffset += PixelsPerCell - FrameString.Len() * Settings.TextSize.X - 2.0f;
				}
				// TextXOffset = FMath::Clamp(TextXOffset, 0.0f, AllottedWidth - Settings.TextSize.X);

				FPaintGeometry TextGeometry = AllottedGeometry.ToPaintGeometry(
					FVector2D(TextXOffset, 1.f),
					Settings.TextSize
				);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					LayerId + 1,
					TextGeometry,
					FrameString,
					FontInfo,
					ESlateDrawEffect::None,
					TextColor
				);
			}
			
			AccumulatedXOffset += PixelsPerCell*NumOfFrames;
		}
		LayerId += 2; // Account for background and text layers
	}
};
	
	

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDopeSheetTimelineSlider::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController)
{
	Controller = InController;

	BackgroundBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.TimelineSlider.Background");
	BackgroundBrush_Active = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.TimelineSlider.Cell.Major");
	BackgroundBrush_Inactive = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.TimelineSlider.Background.Inactive");
	EndCellBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.TimelineSlider.Cell.End");
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SDopeSheetTimelineSlider::OnPaint(const FPaintArgs&        Args, const FGeometry& AllottedGeometry,
                                        const FSlateRect&        MyCullingRect,
                                        FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                        const FWidgetStyle&      InWidgetStyle,
                                        bool                     bParentEnabled) const
{
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
	                                   InWidgetStyle, bParentEnabled);

	if (!Controller || !Controller->CanDraw())
	{
		return LayerId;
	}

	const FSlateRect ViewRect       = Controller->EditableRect;
	OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry.GetLayoutBoundingRect()));

	//////////////////////////////////////////////////////////////////////////
	// *Draw Timeline Background*//
	//////////////////////////////////////////////////////////////////////////
	{
		// Left
		FPaintGeometry BackgroundGeometry =
			AllottedGeometry.ToPaintGeometry(FVector2D(0, 0),
			                                 FVector2D(ViewRect.Left, AllottedGeometry.GetAbsoluteSize().Y));
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId++,
			BackgroundGeometry,
			BackgroundBrush_Inactive,
			ESlateDrawEffect::None
		);
	}

	{
		// Middle
		FPaintGeometry BackgroundGeometry =
			AllottedGeometry.ToPaintGeometry(FVector2D(ViewRect.Left, 0),
			                                 FVector2D(ViewRect.GetSize().X, AllottedGeometry.GetAbsoluteSize().Y));
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId++,
			BackgroundGeometry,
			BackgroundBrush,
			ESlateDrawEffect::None
		);
	}

	{
		// Right
		FPaintGeometry BackgroundGeometry =
			AllottedGeometry.ToPaintGeometry(FVector2D(ViewRect.Right, 0),
			                                 FVector2D(AllottedGeometry.GetAbsoluteSize().X - ViewRect.Right,
			                                           AllottedGeometry.GetAbsoluteSize().Y));
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId++,
			BackgroundGeometry,
			BackgroundBrush_Inactive,
			ESlateDrawEffect::None
		);
	}
	

	

	//////////////////////////////////////////////////////////////////////////
	// *Draw Timeline Frame Text/Ticks*//
	//////////////////////////////////////////////////////////////////////////
	FDopeSheetTimelineRenderer::RenderTimeline(
		OutDrawElements,
		LayerId++,
		AllottedGeometry,
		Controller
	);

	static const float PlayHeadHalfSize = 8.f;

	//////////////////////////////////////////////////////////////////////////
	// Draw hover preview line

	const float TimelineHoverTime = Controller->GetHoverTime();
	if (TimelineHoverTime > 0.f)
	{
		const float LineXPos = FMath::Clamp(AllottedGeometry.Size.X * TimelineHoverTime,
		                                    ViewRect.Left,
		                                    ViewRect.Right);
		const float PlayHeadYOffet = AllottedGeometry.Size.Y - 2.f * PlayHeadHalfSize;

		static const FSlateBrush* PlayHeadPreviewBrush = FMontageGraphEditorStyle::Get().GetBrush(
			"MontageGraph.Timeline.PlayHead.Preview");

		FLinearColor PreviewColor = FLinearColor(1.f, 1.f, 1.f, .5f);

		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(LineXPos - PlayHeadHalfSize, PlayHeadYOffet),
				2.f * FVector2D(PlayHeadHalfSize, PlayHeadHalfSize)),
			PlayHeadPreviewBrush,
			ESlateDrawEffect::None,
			PreviewColor

		);

		TArray<FVector2f> LinePoints;
		LinePoints.AddUninitialized(2);
		LinePoints[0]                = FVector2f(LineXPos, 0.0f);
		LinePoints[1]                = FVector2f(LineXPos, AllottedGeometry.Size.Y - PlayHeadYOffet);
		constexpr float Thickness    = 1.f;
		constexpr float DashLengthPx = 3.f;
		FSlateDrawElement::MakeDashedLines(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(),
			MoveTemp(LinePoints),
			ESlateDrawEffect::None,
			PreviewColor,
			Thickness,
			DashLengthPx
		);
	}


	//////////////////////////////////////////////////////////////////////////
	// Draw PlayHead
	{
		static const FSlateBrush* PlayHeadBrush = FMontageGraphEditorStyle::Get().GetBrush(
			"MontageGraph.Timeline.PlayHead");
		const float LineXPos = Controller->TimeToXOffset(Controller->GetPlayHeadTime(), AllottedGeometry);
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(
				FVector2D(LineXPos - PlayHeadHalfSize, AllottedGeometry.Size.Y - 2.f * PlayHeadHalfSize),
				2.f * FVector2D(PlayHeadHalfSize, PlayHeadHalfSize)),
			PlayHeadBrush,
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}

	OutDrawElements.PopClip();

	return LayerId;
};

void SDopeSheetTimelineSlider::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Controller->SetHoverTime(-1.f);
}

FReply SDopeSheetTimelineSlider::OnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FVector2f MouseLocationAbsolute = InMouseEvent.GetScreenSpacePosition();
	FVector2f MouseLocation         = InGeometry.AbsoluteToLocal(MouseLocationAbsolute);
	float     NewPlaybackTime       = MouseLocation.X / InGeometry.GetLocalSize().X;


	Controller->SetHoverTime(NewPlaybackTime);


	if (Controller->ShouldScrubPlayback())
	{
		// Controller->ScrubPlayback(NewPlaybackTime);
	}

	return FReply::Handled();
}

FReply SDopeSheetTimelineSlider::OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		FVector2f MouseLocationAbsolute = InMouseEvent.GetScreenSpacePosition();
		FVector2f MouseLocation         = InGeometry.AbsoluteToLocal(MouseLocationAbsolute);

		const FSlateRect ViewRect   = Controller->EditableRect;
		const float      SetAtCoord = FMath::Clamp(MouseLocation.X,
		                                           ViewRect.Left, ViewRect.Right);
		Controller->SetPlayHeadTime(Controller->ConvertXCoordToTime(SetAtCoord, InGeometry), true);
		Controller->SetPlaybackState(false);
	}
	return FReply::Handled();
}

FReply SDopeSheetTimelineSlider::OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		Controller->SetPlaybackState(false);
	}
	return FReply::Handled();
}

float SDopeSheetTimelineSlider::GetScreenSpaceXOffsetFromTime(const float      InTime,
                                                              const FGeometry& AllottedGeometry) const
{
	return AllottedGeometry.Size.X * (InTime - Controller->ViewStartTime);
}

