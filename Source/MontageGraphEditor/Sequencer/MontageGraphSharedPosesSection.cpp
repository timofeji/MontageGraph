// Fill out your copyright notice in the Description page of Project Settings.


#include "MontageGraphSharedPosesSection.h"

#include "EntitySystem/BuiltInComponentTypes.h"


USharedPoseSection::USharedPoseSection(const FObjectInitializer& Init)
{
}

void USharedPoseSection::ImportEntityImpl(UMovieSceneEntitySystemLinker* EntityLinker,
                                          const FEntityImportParams& Params, FImportedEntity* OutImportedEntity)
{
	using namespace UE::MovieScene;
	//
	// FMovieSceneTrackInstanceComponent TrackInstance{
	// 	decltype(FMovieSceneTrackInstanceComponent::Owner)(this), UMovieSceneCVarTrackInstance::StaticClass()
	// };
	//
	// OutImportedEntity->AddBuilder(
	// 	FEntityBuilder()
	// 	.AddTag(FBuiltInComponentTypes::Get()->Tags.Root)
	// 	.Add(FBuiltInComponentTypes::Get()->TrackInstance, TrackInstance)
	// );
}
