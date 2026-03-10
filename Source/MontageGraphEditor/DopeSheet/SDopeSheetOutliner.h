#pragma once

#include "CoreMinimal.h"
#include "Tracks/DopeSheetTrackViewModel.h"


DECLARE_DELEGATE_TwoParams(FOnItemExpansionStateChanged, FDopeSheetTrackViewModelRef, bool)

class SDopeSheetOutliner : public STreeView<FDopeSheetTrackViewModelRef>
{
	SLATE_BEGIN_ARGS(SDopeSheetOutliner)
		{
		}

		SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalScrollbar)
	SLATE_END_ARGS()

	bool IsDeleteEnabled() const;
	void   Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController);
	FReply OnAddTrackClicked(FDopeSheetTrackViewModelRef ParentTrack);
	FReply OnRemoveTrackClicked();

	TSharedRef<ITableRow> MakeTableRowWidget(FDopeSheetTrackViewModelRef       InItem,
	                                         const TSharedRef<STableViewBase>& TableViewBase);
	void HandleGetChildren(FDopeSheetTrackViewModelRef InItem, TArray<FDopeSheetTrackViewModelRef>& OutChildren);

	void SetExpansionRecursive(FDopeSheetTrackViewModelRef InItem, bool bShouldBeExpanded);
	void OnExpansionChanged(TSharedRef<FDopeSheetTrackViewModel> ItemExpanded, bool bArg);

	void SetFilterText(const FText& Text);

	void OnAddTrackSelected(UClass* TrackClass, FDopeSheetTrackViewModelRef ObjPtr);

	FReply HandleMouseClick();

	TSharedPtr<SWidget> OnContextMenuOpening();
	void AssignSelectedTracksToCollection(FName CollectionName);
	void OnAddRootTrackSelected(UClass* TrackClass);

private:

	TSharedPtr<FDopeSheetController>                   Controller;
	TSharedPtr<STreeView<FDopeSheetTrackViewModelRef>> TreeView;
};
