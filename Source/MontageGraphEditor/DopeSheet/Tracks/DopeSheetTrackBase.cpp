// Copyright Drop Games Inc.


#include "DopeSheetTrackBase.h"

#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"


TSharedRef<SWidget> UDopeSheetTrackBase::MakeTrackTimelineWidget(TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
{
	return SNew(SDopeSheetTrackTimeline, TrackModelPtr);
}

const FText UDopeSheetTrackBase::GetTrackName() const
{
	return FText::FromString("New DopeSheet Track");
}

const FLinearColor UDopeSheetTrackBase::GetTrackColor() const
{
	return FColor(169, 159, 129, 155);
}

FSlateIcon UDopeSheetTrackBase::GetTrackIcon() const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Sequencer.Tracks.Sub");
}

void UDopeSheetTrackBase::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	OnTrackPropertiesChanged.Broadcast();
}

void UDopeSheetTrackBase::KeyFrameRange(int SelectionStartFrame, int SelectionEndFrame)
{
	
}

void UDopeSheetTrackBase::KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame)
{
	
}
