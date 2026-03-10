#pragma once
#include "CoreMinimal.h"

class UDopeSheetTrackSection;
class FDopeSheetController;
class UDopeSheetTrackBase;


typedef TSharedRef<class FDopeSheetTrackViewModel> FDopeSheetTrackViewModelRef;
class MONTAGEGRAPHEDITOR_API FDopeSheetSectionViewModel  : public TSharedFromThis<FDopeSheetSectionViewModel>
{
public:
	double StartTime;
	double EndTime;

	UDopeSheetTrackSection* Section;
	FDopeSheetTrackViewModel* TrackModelPtr;


	FDopeSheetSectionViewModel()
	{
	}

	FDopeSheetSectionViewModel(float InStartTime, float InEndTime)
		: StartTime(InStartTime), EndTime(InEndTime)
	{
	}

	FDopeSheetSectionViewModel(UDopeSheetTrackSection* InSection, FDopeSheetTrackViewModel* TrackModel);
	
	double GetTimeLength() const { return EndTime - StartTime; }
	
	bool IsSelected();
	void CommitTimeRange();
};


class MONTAGEGRAPHEDITOR_API FDopeSheetTrackViewModel : public TSharedFromThis<FDopeSheetTrackViewModel>
{
public:
	UDopeSheetTrackBase*             ObjPtr;
	UDopeSheetTrackBase*             ParentPtr;
	TSharedPtr<FDopeSheetController> Controller;

	TArray<TSharedPtr<FDopeSheetSectionViewModel>> SectionModels;

	FDopeSheetTrackViewModel(UDopeSheetTrackBase* InObjPtr, TSharedPtr<FDopeSheetController> InController);

	virtual ~FDopeSheetTrackViewModel();

	TSharedRef<SWidget> MakeTrackTimelineWidget();
	
	
	void DeleteSection(int32 SectionIndex);
	void ShiftSection(int32 SectionIndex, double MoveTimeDelta);

	void  SetTrackHeight(float NewTrackHeight);
	float GetTrackHeight();

	bool IsVisible();
	
	FLinearColor GetTrackColor();

	TArray<FDopeSheetTrackViewModelRef> Children;

	/** Time offset (in seconds) applied when laying out this track in horizontal comparison mode. */
	double DisplayTimeOffset = 0.0;

	virtual void DroppedAssetsOnTrack(TArray<FAssetData> Array);
};
