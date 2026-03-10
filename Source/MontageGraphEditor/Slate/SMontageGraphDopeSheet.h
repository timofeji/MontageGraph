// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "DopeSheet/SAnimDopeSheet.h"
#include "Widgets/SCompoundWidget.h"



/**
 * 
 */
class MONTAGEGRAPHEDITOR_API SMontageGraphDopeSheet : public SCompoundWidget
{
public:
	DECLARE_DELEGATE(FOnDopeSheetUpdated)
	SLATE_BEGIN_ARGS(SMontageGraphDopeSheet)
		{
		}

		SLATE_EVENT(FOnDopeSheetUpdated, OnUpdateNodes)
	SLATE_END_ARGS()

public:
	void SetSelection(class UMGEdNode_Montage* NewSelection);
	void SetMultiSelection(const TArray<TArray<class UMGEdNode_Montage*>>& OrderedChains);

	/** Constructs this widget with InArgs */
	void  Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> TimelineController);
	virtual int32 OnPaint(const FPaintArgs&        Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	              FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle&        InWidgetStyle,
	              bool                     bParentEnabled) const override;


	TSharedPtr<FDopeSheetController> Controller;
	TSharedPtr<SAnimDopeSheet>       DopeSheetWidget;

	void                             OnMontageSequencesUpdated();
	void                             SetMontageCellViews();

	/** Re-applies current layout mode without rebuilding selection (call after toggling bHorizontalLayout). */
	void RefreshLayout();

	/** Sets DisplayTimeOffset on each track's ViewModel so node B sections appear to the right of node A. */
	void RefreshTrackOffsets();

	FOnDopeSheetUpdated OnUpdateNodes;

	/** When true, selected nodes are shown side-by-side horizontally with blend-time overlap. */
	bool bHorizontalLayout = false;

protected:
	TArray<UMGEdNode_Montage*>                      SelectedNodes;
	TArray<UDopeSheetTrackBase*>                     CombinedTracks;

};
