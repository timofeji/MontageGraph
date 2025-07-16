// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "IDopeSheetTrack.h"
#include "DopeSheet/DopeSheetController.h"
#include "UObject/Object.h"
#include "DopeSheetTrackBase.generated.h"


USTRUCT()
struct FDopeSheetKey
{
	GENERATED_BODY()
	
	FDopeSheetKey(){
	
	}
	
};

#if WITH_EDITOR
UCLASS(Abstract)
class MONTAGEGRAPHEDITOR_API UDopeSheetTrackSection : public UObject
{
	GENERATED_BODY()
public:	
	UPROPERTY()
	float StartTime;
	
	UPROPERTY()
	float EndTime;
	
};

/**
 * 
 */
UCLASS(Abstract)
class MONTAGEGRAPHEDITOR_API UDopeSheetTrackBase : public UObject, public IDopeSheetTrack
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UDopeSheetTrackBase*> SubTracks;

	UPROPERTY()
	TArray<UDopeSheetTrackSection*> Sections;
	
	UPROPERTY()
	TArray<FDopeSheetKey> Keys;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	bool bIsExpanded = true;

	UPROPERTY()
	bool bShouldDrawCells = true;
	
	UPROPERTY()
	float TrackHeight = 32.f;
#endif

	template <class DataAssetType>
	DataAssetType* CreateNewDataObject(UObject* Outer, FName Name)
	{
		UObject* NewData = GenerateNewDataAsset(Outer, Name);
		return reinterpret_cast<DataAssetType*>(NewData);
	}

	virtual UObject* GenerateNewDataAsset(UObject* Outer, FName Name) { return nullptr; }

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
#if WITH_EDITORONLY_DATA

	virtual void KeyFrameRange(int SelectionStartFrame, int SelectionEndFrame);
	virtual void KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame);
	
	/*Begin IDopeSheetTrack Interface*/
	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const override;
	virtual const FText         GetTrackName() const override;
	virtual const FLinearColor  GetTrackColor() const override;
	virtual FSlateIcon          GetTrackIcon() const override;
	/*End IDopeSheetTrack Interface*/
	
	float GetTrackHeight() const { return TrackHeight; }
#endif
	
};

#endif