#pragma once
#include "CoreMinimal.h"

class FDopeSheetController;
class UDopeSheetTrackBase;

typedef TSharedRef<class FDopeSheetTrackViewModel> FDopeSheetTrackViewModelRef;

struct FDopeSheetSectionViewModel 
{
	float StartTime;
	float EndTime;


	FDopeSheetSectionViewModel()
	{
	}

	FDopeSheetSectionViewModel(float InStartTime, float InEndTime)
		: StartTime(InStartTime), EndTime(InEndTime)
	{
	}

	FDopeSheetSectionViewModel(FFloatRange TimeSpan)
		: StartTime(TimeSpan.GetLowerBoundValue()), EndTime(TimeSpan.GetUpperBoundValue())
	{
	}

	double GetTimeLength() const { return EndTime - StartTime; }
};


class MONTAGEGRAPHEDITOR_API FDopeSheetTrackViewModel : public TSharedFromThis<FDopeSheetTrackViewModel>
{
public:
	UDopeSheetTrackBase*             ObjPtr;
	TSharedPtr<FDopeSheetController> Controller;

	TArray<FDopeSheetSectionViewModel> SectionModels;

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

	virtual void DroppedAssetsOnTrack(TArray<FAssetData> Array);
};
