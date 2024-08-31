#include "MontageGraphSharedPosesTrackEditor.h"

#include "MontageGraphSharedPosesTrack.h"
#include "ISequencer.h"
#include "MontageGraphEditorStyles.h"
#include "SequencerSectionPainter.h"
#include "TimeToPixel.h"
#include "Fonts/FontMeasure.h"


namespace SharedPosesTrackConstants
{
	/** The minimum amount of pixels between each major ticks on the widget */
	const int32 MinPixelsPerDisplayTick = 12;

	/**The smallest number of units between between major tick marks */
	const float MinDisplayTickSpacing = 0.001f;

	/**The fraction of the current view range to scroll per unit delta  */
	const float ScrollPanFraction = 0.1f;

	/** Marked frame label box margin */
	const int32 MarkLabelBoxMargin = 5;

	/** Marked frame label box margin on the opposite side of the marker time */
	const int32 MarkLabelBoxWideMargin = 6;
}


#define LOCTEXT_NAMESPACE "FMontageGraphSharedPosesTrackEditor"

FSharedPoseSection::FSharedPoseSection(TWeakPtr<ISequencer> InSequencer, UMovieSceneSection& InSectionObject)
	: FSequencerSection(InSectionObject), WeakSequencer(InSequencer)
{
	FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService().ToSharedPtr();
	SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);
	// SmallBoldLayoutFont = FCoreStyle::GetDefaultFontStyle("Bold", 10);
}

int32 FSharedPoseSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 LayerId = FSequencerSection::OnPaintSection(Painter);

	const FTimeToPixel& TimeConverter = Painter.GetTimeConverter();
	const float Position = TimeConverter.FrameToPixel(FFrameNumber(300));
	
	FLinearColor DrawColor =
		FLinearColor::Red;

	// double Seconds = MarkedFrame.FrameNumber / GetTickResolution();
	// float MarkPixel = RangeToScreen.InputToLocalX(Seconds);

	const FString& LabelString = "Combo1_End";
	FVector2D TextSize = FontMeasureService->Measure(LabelString, SmallLayoutFont);

	// Flip the text position if getting near the end of the view range
	bool bDrawLeft = (Painter.SectionGeometry.Size.X - Position) < (TextSize.X + SharedPosesTrackConstants::MarkLabelBoxMargin +
		SharedPosesTrackConstants::MarkLabelBoxWideMargin);
	float TextPosition = bDrawLeft
		                     ? Position - TextSize.X - SharedPosesTrackConstants::MarkLabelBoxMargin
		                     : Position + SharedPosesTrackConstants::MarkLabelBoxMargin;

	FVector2D LabelPosition = FVector2D(TextPosition, 0);
	FVector2D LabelSize  = TextSize;


	const FSlateBrush* LabelBrush = bDrawLeft
		                                ? FAppStyle::GetBrush("Sequencer.MarkedFrame.LabelLeft")
		                                : FAppStyle::GetBrush("Sequencer.MarkedFrame.LabelRight");


	FSlateDrawElement::MakeBox(
		Painter.DrawElements,
		LayerId + 1,
		Painter.SectionGeometry.ToPaintGeometry(
			TextSize + SharedPosesTrackConstants::MarkLabelBoxMargin +
			SharedPosesTrackConstants::MarkLabelBoxWideMargin,
			FSlateLayoutTransform(FVector2D(Position, 0.f))),
		LabelBrush,
		ESlateDrawEffect::None,
		DrawColor.CopyWithNewOpacity(0.23f)
	);

	FSlateDrawElement::MakeText(
		Painter.DrawElements,
		LayerId + 2,
		Painter.SectionGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(LabelPosition)),
		LabelString,
		/*(bIsHovered || bIsSelected) ? SmallBoldLayoutFont :*/
		SmallLayoutFont,
		ESlateDrawEffect::None,
		FLinearColor::White	
	);
	

	FVector2D StartPoint = (FVector2D(Position, 0.f)); // Start of the line
	FVector2D EndPoint = (FVector2D(Position, Painter.SectionGeometry.Size.Y)); // End of the line


	TArray<FVector2D> LinePoints;

	// Calculate the total length of the line
	float TotalLength = (EndPoint - StartPoint).Size();

	// Calculate the unit direction vector of the line
	FVector2D Direction = (EndPoint - StartPoint).GetSafeNormal();

	// Initialize the current position
	FVector2D CurrentPosition = StartPoint;

	// Add points for each segment
	bool bDraw = true;
	while ((CurrentPosition - StartPoint).Size() < TotalLength)
	{
		// Determine the next point position
		FVector2D NextPoint = CurrentPosition + (bDraw ? 6.f : 15.f) * Direction;

		// If drawing, add the segment to the array
		if (bDraw)
		{
			LinePoints.Add(CurrentPosition);
			LinePoints.Add(NextPoint);
		}

		// Move to the next position
		CurrentPosition = NextPoint;

		// Toggle between drawing and skipping
		bDraw = !bDraw;
	}


	// Draw the line
	FSlateDrawElement::MakeLines(
		Painter.DrawElements,
		LayerId + 1, // Draw on top of other elements
		Painter.SectionGeometry.ToPaintGeometry(),
		LinePoints,
		ESlateDrawEffect::None,
		DrawColor,
		true, // Anti-alias
		4.f // Thickness
	);

	return LayerId + 1;
}


FSharedPosesTrackEditor::FSharedPosesTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

void FSharedPosesTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddSharedPosesTrack", "Add Shared Poses Track"), //Label
		LOCTEXT("AddSharedPosesTrack", "Add Shared Poses Track"), //Tooltip
		FSlateIcon(FMontageGraphEditorStyles::Get().GetStyleSetName(), "MontageGraph.PoseLink"),
		FUIAction(FExecuteAction::CreateLambda([this]
		{
			UMovieScene* FocusedMovieScene = GetFocusedMovieScene();
			if (IsValid(FocusedMovieScene))
			{
				//Create scoped transaction:
				const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "AddSharedPoses_Transaction",
				                                               "Add Shared Poses Track"));
				FocusedMovieScene->Modify();

				UMontageGraphSharedPosesTrack* NewTrack = FocusedMovieScene->AddTrack<UMontageGraphSharedPosesTrack>();
				ensure(NewTrack);

				// FSharedPosesTrackEditor::AddNewTrack(NewTrack, FocusedMovieScene);


				//Notify SequencerAboutTransactions:
				GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
			}
		})));
}

bool FSharedPosesTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return true;
}

TSharedRef<ISequencerTrackEditor> FSharedPosesTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> Sequencer)
{
	return MakeShareable(new FSharedPosesTrackEditor(Sequencer));
}


TSharedRef<ISequencerSection> FSharedPosesTrackEditor::MakeSectionInterface(
	UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	check(SupportsType(SectionObject.GetOuter()->GetClass()));

	return MakeShareable(new FSharedPoseSection(GetSequencer(), SectionObject));
}
#undef LOCTEXT_NAMESPACE
