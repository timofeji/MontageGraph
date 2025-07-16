#pragma once
#include "CoreMinimal.h"
#include "Tracks/DopeSheetTrackViewModel.h"

#if WITH_EDITOR
struct FDopeSheetViewSection
{
	int   NumOfFrames = 60;
	float StartTime   = 0.f;
	float EndTime     = 1.f;

	FDopeSheetViewSection(int InFrameRate, float InStart, float InEnd) :
		NumOfFrames(InFrameRate),
		StartTime(InStart),
		EndTime(InEnd)
	{
	}
};

enum class EDopeSheetSelectionFlags : uint8
{
	None = 0,

	/** We're currently selecting(Draw selection Box)*/
	Selecting = (1 << 0),

	/** We've commited our selection (Draw box around selected frames)*/
	RangeSelected = (1 << 1),

	/** If true, we don't snap our selection to the nearest cell*/
	BoxSelect = (1 << 2),
};

ENUM_CLASS_FLAGS(EDopeSheetSelectionFlags);


enum class EDopeSheetFlags : uint32
{
	None = 0,

	/** Can't draw/ Our Cache is Invalidated */
	Stale = (1 << 0),

	//////////////////////////////////////////////////
	// Horizontal viewport flags

	/** Width has changed. */
	HViewChanged = (1 << 1),

	VLayoutChanged = (1 << 2),

	/** We received a new tracks source*/
	RebuildTracks = (1 << 8),


	//////////////////////////////////////////////////
	AllHorizontal = HViewChanged,
	AllVertical   = RebuildTracks | VLayoutChanged,
	All           = AllHorizontal | AllVertical
};

ENUM_CLASS_FLAGS(EDopeSheetFlags);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDopeSheetControllerUpdated, EDopeSheetFlags);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDopeSheetTogglePlayback, bool)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDopeSheetTimeChanged, float)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDopeSheetSectionSelected, UObject*)


class FDopeSheetController : public TSharedFromThis<FDopeSheetController>
{
public:
	float ViewStartTime      = 0.f;
	float ViewEndTime        = 1.f;
	float TimeDurationInView = 1.f;


	void UpdateViewForGeometry(const FGeometry& Geometry);

	bool CanDraw();
	bool CanUpdate();

	//*Rectangle representing all the CellViews on the X-axis, and Tracks on the Y-axis*//
	FSlateRect EditableRect;

	TArray<const float> CachedTrackHeights;
	TArray<const float> CachedSectionCellWidths;


	//TODO: This shoudnt be an attribtue
	TAttribute<float> TimeSliderHeight = 32.f;

	TArray<FDopeSheetViewSection> Sections;

	void AddViewSection(FDopeSheetViewSection NewSection);
	void Reset(bool bMarkDirty = false);


	/////////////////////////////////////////////////////////////////////////
	/// NAVIGATION
	/////////////////////////////////////////////////////////////////////////
	///
	FReply OnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, TSharedRef<SWidget> InWidget);
	FReply OnMouseButtonDown(const FGeometry&    MyGeometry, const FPointerEvent& MouseEvent,
	                         TSharedRef<SWidget> InWidget);

	virtual void CreateContextMenu(const FPointerEvent& MouseEvent, TSharedRef<SWidget> InWidget);

	bool bIsPanning     = false;
	bool bIsLMB_Pressed = false;
	bool bIsRMB_Pressed = false;


	void ZoomInView(float Delta, float Center);
	void ScrollView(float MoveDelta);

	void SetViewRange(double NewStartTime, double NewEndTime);
	void FrameViewRange();

	float LocalXCoordToTime(float XCoord) const;
	float AbsoluteXCoordToTime(float XCoord) const;

	float TimeToXOffset(const float InTime, const FGeometry& AllottedGeometry) const;
	float ConvertXCoordToTime(float XCoord, const FGeometry& AllottedGeometry) const;

	//*Regenerated View Cache*//

	void SetPlaybackState(bool bNewState);
	void HoldPlayback(bool bShouldPlay);
	bool ShouldScrubPlayback();
	void ScrubPlayback(float NewPlaybackTime);

	bool bIsPlayingAnimation = false;
	bool bPlayBackBeforeHoldState;

	FOnDopeSheetTogglePlayback    OnTogglePlayback;
	FOnDopeSheetTimeChanged       OnTimeChanged;
	FOnDopeSheetControllerUpdated OnUpdate;

	FOnDopeSheetSectionSelected OnSectionSelected;

	//////////////////////////////////////////////////////
	/// Keying
	//////////////////////////////////////////////////////
	void KeySelection();

	//////////////////////////////////////////////////////
	/// SELECTION
	//////////////////////////////////////////////////////
	void SetSelectionFlags(EDopeSheetSelectionFlags Selecting);
	bool HasSelectionFlags(EDopeSheetSelectionFlags Selecting) const;

	void BeginSelection(const FGeometry& Geometry, const FPointerEvent& PointerEvent);
	void DragSelection(const FGeometry& Geometry, const FPointerEvent& PointerEvent);
	void EndSelection();

	void ClearSelection();

	void           ProcessBoxSelection();
	FPaintGeometry GetAdjustedSelectionGeometry(const FGeometry& InGeometry) const;


	void DrawSelection(FSlateWindowElementList& OutDrawElements, uint32 LayerID, const FGeometry& Geometry) const;

	void  SetHoverTime(float NewTime) { HoverTime = NewTime; }
	float GetHoverTime() { return HoverTime; }

	void  SetPlayHeadTime(float NewTime, bool bBroadcastChange);
	float GetPlayHeadTime() { return PlayHeadTime; }

	int TotalNumberOfFrames = 0;

protected:
	float HoverTime    = 0.0f;
	float PlayHeadTime = 0.0f;


	FVector2D StartCoord;
	FVector2D EndCoord;

	int SelectionStartRow   = -1;
	int SelectionEndRow     = -1;
	int SelectionStartFrame = -1;
	int SelectionEndFrame   = -1;

	float SelectionWidth = 0;

	EDopeSheetSelectionFlags SelectionFlags;

public:
	void OnTrackExpanded(TSharedRef<FDopeSheetTrackViewModel> ExpandedTrackModel, bool bExpanded);
	TArray<UDopeSheetTrackBase*> VisibleTracks;

	void                                SetTracksSource(const TArray<class UDopeSheetTrackBase*>& TrackSrc);
	TArray<FDopeSheetTrackViewModelRef> RootTracks;

	void ForceUpdate();
	void AddDirtyFlags(EDopeSheetFlags NewFlags);

private:
	EDopeSheetFlags DirtyFlags;
	
	FGeometry   CachedGeometry;
};
#endif
