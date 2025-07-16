// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "UObject/Object.h"
#include "MontageTrack_BlendLinks.generated.h"

/**
 * 
 */

#if WITH_EDITOR
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_BlendLinks : public UDopeSheetTrackBase
{
	GENERATED_BODY()
	
	UMontageTrack_BlendLinks (const FObjectInitializer& ObjectInitializer);
	

	/*Begin IDopeSheetTrack Interface*/
	virtual const FText         GetTrackName() const override;
	virtual const FLinearColor  GetTrackColor() const override;
	virtual FSlateIcon          GetTrackIcon() const override;
	/*End IDopeSheetTrack Interface*/

};
#endif
