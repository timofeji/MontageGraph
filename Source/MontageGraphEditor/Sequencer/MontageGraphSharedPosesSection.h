// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ISequencerSection.h"
#include "EntitySystem/IMovieSceneEntityProvider.h"
#include "UObject/Object.h"
#include "MontageGraphSharedPosesSection.generated.h"


UCLASS(MinimalAPI)
class USharedPoseSection 
	: public UMovieSceneSection
	, public IMovieSceneEntityProvider
{
	GENERATED_BODY()

public:
	/** Constructs a new console variable section */
	USharedPoseSection(const FObjectInitializer& Init);

private:
	
	virtual void ImportEntityImpl(UMovieSceneEntitySystemLinker* EntityLinker, const FEntityImportParams& Params, FImportedEntity* OutImportedEntity) override;
};

