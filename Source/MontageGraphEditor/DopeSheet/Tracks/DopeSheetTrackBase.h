// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "IDopeSheetTrack.h"
#include "DopeSheet/DopeSheetController.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"
#include "UObject/Object.h"
#include "DopeSheetTrackBase.generated.h"

class UMontageGraph;
class FMenuBuilder;


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
	/*Section StartTime, relative to Montage Beginning*/
	UPROPERTY()
	double StartTime;
	
	/*Section EndTime, relative to Montage Beginning*/
	UPROPERTY()
	double EndTime;
	
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
	bool bAllowRename  = true;

	UPROPERTY()
	bool bAllowDelete  = true;

	UPROPERTY()
	float TrackHeight = 32.f;

	/** Collection this track belongs to. Tracks with the same name are grouped/hidden together. NAME_None = no collection. */
	UPROPERTY()
	FName CollectionName = NAME_None;
#endif

	template <class DataAssetType>
	DataAssetType* CreateNewDataObject(UObject* Outer, FName Name)
	{
		UObject* NewData = GenerateNewDataAsset(Outer, Name);
		return reinterpret_cast<DataAssetType*>(NewData);
	}

	virtual UObject* GenerateNewDataAsset(UObject* Outer, FName Name) { return nullptr; }

	/**
	 * Bake this track's data into BakedData for the given runtime node.
	 *
	 * Called once per track during graph compilation.  Override in each concrete track class to
	 * write your payload — use BakedData.Montage / CollisionTracer for the core slots, or
	 * BakedData.ExtendedPayloads[FName("MyKey")] for arbitrary future data.
	 * Default implementation is a no-op so tracks that produce no baked data need not override.
	 */
	virtual void BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
	                        UMontageGraph* OwnerGraph, const FString& DisplayName) {}

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	
	
	virtual bool CanCreateSubTracks();
	
#if WITH_EDITORONLY_DATA

	virtual void KeyFrameRange(int SelectionStartFrame, int SelectionEndFrame);
	virtual void KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame);
	
	/*Begin IDopeSheetTrack Interface*/
	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const override;
	virtual const FText         GetTrackName() const override;
	virtual const FLinearColor  GetTrackColor() const override;
	virtual const FLinearColor  GetSectionColor() const override;
	virtual FSlateIcon          GetTrackIcon() const override;
	/*End IDopeSheetTrack Interface*/
	
	float GetTrackHeight() const { return TrackHeight; }
	virtual void GetSubTrackClasses(TArray<UClass*>& TrackClasses) {};

	/** Override to add track-specific entries to the outliner right-click context menu. */
	virtual void BuildContextMenuActions(FMenuBuilder& MenuBuilder) {}

#endif
	
};

#endif