#pragma once
#include "MovieSceneTrackEditor.h"

class FSharedPoseSection
	: public FSequencerSection
{
public:
	TSharedPtr<FSlateFontMeasure, ESPMode::ThreadSafe> FontMeasureService;
	FSlateFontInfo SmallLayoutFont;

	FSharedPoseSection(TWeakPtr<ISequencer> InSequencer, UMovieSceneSection& InSectionObject);
		
	

	virtual int32 OnPaintSection(FSequencerSectionPainter& Painter) const override;
	
	TWeakPtr<ISequencer> WeakSequencer;
};


class FSharedPosesTrackEditor
	: public FMovieSceneTrackEditor
{
	
public:
	FSharedPosesTrackEditor(TSharedRef<ISequencer> InSequencer);


	virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject,
	                                                           UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> Sequencer);
	
};
