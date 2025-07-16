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

	/** Constructs this widget with InArgs */
	void  Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> TimelineController);
	virtual int32 OnPaint(const FPaintArgs&        Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	              FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle&        InWidgetStyle,
	              bool                     bParentEnabled) const override;


	TSharedPtr<FDopeSheetController> Controller;
	TSharedPtr<SAnimDopeSheet>       DopeSheetWidget;

	void                             OnMontageSequencesUpdated();
	void                             SetMontageCellViews();
	
	 FOnDopeSheetUpdated OnUpdateNodes;
protected:	
	UMGEdNode_Montage*                              SelectedNode;
	
};
