// Fill out your copyright notice in the Description page of Project Settings.


#include "MontageGraphSharedPosesTrack.h"

#include "MontageGraphSharedPosesTrackEditor.h"
#include "MovieSceneCommonHelpers.h"

#define LOCTEXT_NAMESPACE "MontageGraphSharedPosesTrack"

UMontageGraphSharedPosesTrack::UMontageGraphSharedPosesTrack( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
#if WITH_EDITORONLY_DATA
	TrackTint = FColor(23, 89, 145, 65);
#endif

	// By default, don't evaluate cvar values in pre and postroll as that would conflict with
	// the previous shot's desired values.
	EvalOptions.bEvaluateInPreroll = EvalOptions.bEvaluateInPostroll = false;
	// SupportedBlendTypes.Add(EMovieSceneBlendType::Absolute);
}

/* UMovieSceneTrack interface
 *****************************************************************************/

void UMontageGraphSharedPosesTrack::AddSection(UMovieSceneSection& Section)
{
	if (USharedPoseSection* CVarSection = Cast<USharedPoseSection>(&Section))
	{
		Sections.Add(CVarSection);
	}
}

bool UMontageGraphSharedPosesTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == USharedPoseSection::StaticClass();
}

UMovieSceneSection* UMontageGraphSharedPosesTrack::CreateNewSection()
{
	return NewObject<USharedPoseSection>(this, NAME_None, RF_Transactional);
}

bool UMontageGraphSharedPosesTrack::SupportsMultipleRows() const
{
	return true;
}

EMovieSceneTrackEasingSupportFlags UMontageGraphSharedPosesTrack::SupportsEasing(FMovieSceneSupportsEasingParams& Params) const
{
	return EMovieSceneTrackEasingSupportFlags::None;
}

const TArray<UMovieSceneSection*>& UMontageGraphSharedPosesTrack::GetAllSections() const
{
	return Sections;
}

bool UMontageGraphSharedPosesTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}


bool UMontageGraphSharedPosesTrack::IsEmpty() const
{
	return Sections.Num() == 0;
}

void UMontageGraphSharedPosesTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}

void UMontageGraphSharedPosesTrack::RemoveSectionAt(int32 SectionIndex)
{
	UMovieSceneSection* SectionToDelete = Sections[SectionIndex];
	Sections.RemoveAt(SectionIndex);
	MovieSceneHelpers::SortConsecutiveSections(MutableView(Sections));
}

void UMontageGraphSharedPosesTrack::RemoveAllAnimationData()
{
	Sections.Empty();
}

#if WITH_EDITORONLY_DATA
FText UMontageGraphSharedPosesTrack::GetDefaultDisplayName() const
{
	return LOCTEXT("TrackName", "SharedPoses");
}
#endif

#undef LOCTEXT_NAMESPACE

