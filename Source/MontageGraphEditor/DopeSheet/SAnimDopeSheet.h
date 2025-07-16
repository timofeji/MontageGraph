// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "SDopeSheetOutliner.h"
#include "Widgets/SCompoundWidget.h"
#include "Windows/AllowWindowsPlatformTypes.h"


class FDopeSheetController;
class FDopeSheetTrackViewModel;


class FDopeSheetFrameCellRenderer
{
	
public:
	FDopeSheetFrameCellRenderer();
	inline void Draw(FSlateWindowElementList&                OutDrawElements,
	            int32                                   LayerId,
	            const FGeometry&                        AllottedGeometry,
	            const TSharedPtr<FDopeSheetController>& Controller) const;
	
	void Update(const TSharedPtr<FDopeSheetController>& Controller, const FGeometry& Geometry);

private:
	const FSlateBrush* CellBrush;
	const FSlateBrush* CellBrush_Hover;


	TArray<FVector2f> FrameGeometries;
	FLinearColor      LineColor;
};



class SDopeSheet : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDopeSheet)
	{
	}
	SLATE_END_ARGS()
	
	


	void           Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController);
	virtual int32  OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& MouseEvent) override;

	void                             RebuildTracks();


protected:
	TSharedPtr<SVerticalBox> TrackVerticalBox;
	
private:
	TSharedPtr<FDopeSheetController> Controller;
	FDopeSheetFrameCellRenderer CellRenderer;

	
	const FSlateBrush* VoidBrush;
};


class SAnimDopeSheet : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAnimDopeSheet)
		{
		}
	SLATE_END_ARGS()


	void KeySelection();
	bool CanKeySelection();
	
	
	void CreateDopeSheetCommands();
	
	void Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController);
	
	
	void OnOutlinerFilterTextChanged(const FText& Text);

	void RequestRefresh();
	
	void SetTracksSource(const TArray<class UDopeSheetTrackBase*>& Array);

//////////////////////////////////////////////////////////////////
/// UI COMMANDS
//////////////////////////////////////////////////////////////////
	void TogglePlayback();
	
	void ResetView() const;
	
	void FindTrack();

protected:
	
	TSharedPtr<FUICommandList> DopeSheetActions;
	
	
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	
	virtual bool   SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
//////////////////////////////////////////////////////////////////

	TSharedPtr<SDopeSheetOutliner>   OutlinerWidget;
	TSharedPtr<SSearchBox>           OutlinerFilterBox;
	TSharedPtr<SVerticalBox>         TrackVerticalBox;
	TSharedPtr<SDopeSheet>           DopeSheet;

	TSharedPtr<FDopeSheetController> Controller;

};


