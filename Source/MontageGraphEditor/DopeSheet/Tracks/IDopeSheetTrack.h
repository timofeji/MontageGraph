#pragma once

#include "IDopeSheetTrack.generated.h"


class FDopeSheetTrackViewModel;

UINTERFACE(BlueprintType, meta=(CannotImplementInterfaceInBlueprint), MinimalAPI)
class UDopeSheetTrack: public UInterface
{
	GENERATED_BODY()
};


DECLARE_MULTICAST_DELEGATE(FTrackDataUpdatedDelegate);

class IDopeSheetTrack
{		
public:
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
//////////////////////////////////////////////////////////////////////////////////
///* Timeline
	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
	{
		return SNullWidget::NullWidget;
	};
	
	virtual void DroppedAssetsOnTrack(TArray<FAssetData> AssetArr){};

	
	// ** Gets the Track Label/Title */
	virtual const FText GetTrackName() const = 0;
	
	// ** Gets the Track Color*/
	virtual const FLinearColor GetTrackColor() const = 0;

	
	// ** Gets the Section Color usually accented*/
	virtual const FLinearColor  GetSectionColor() const = 0;
	
	// ** Gets the Track Icon*/
	virtual FSlateIcon GetTrackIcon() const = 0;

	/*Used for updating widgets when the object data changed*/
	FTrackDataUpdatedDelegate OnTrackPropertiesChanged;
#endif
};


