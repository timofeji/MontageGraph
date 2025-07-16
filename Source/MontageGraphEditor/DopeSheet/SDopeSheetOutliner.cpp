#include "SDopeSheetOutliner.h"

#include "DopeSheetController.h"
#include "Tracks/DopeSheetTrackBase.h"
#include "Tracks/DopeSheetTrackViewModel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Views/STreeView.h"


class SDopeSheetOutlinerRow : public STableRow<FDopeSheetTrackViewModelRef>
{
	SLATE_BEGIN_ARGS(SDopeSheetOutlinerRow)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments&           InArgs, const TSharedRef<STableViewBase>& InOwnerTable,
	               FDopeSheetTrackViewModelRef TrackModel)
	{
		STableRow<FDopeSheetTrackViewModelRef>::Construct(
			STableRow<FDopeSheetTrackViewModelRef>::FArguments()
			.Padding(0.0f),
			InOwnerTable);

		UDopeSheetTrackBase* TrackPtr = TrackModel->ObjPtr;
		if (!TrackPtr) { return; }


		ChildSlot
		[
			SNew(SBox)
			.MinDesiredHeight(TrackModel->GetTrackHeight())
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TrackPtr->GetTrackColor().Desaturate(.75f))
				.Padding(0.f)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.MinWidth(15.f)
					.MaxWidth(15.f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("WhiteBrush"))
						.ColorAndOpacity(TrackPtr->GetTrackColor())
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Fill)
					.Padding(2.0f, 0.f)
					[
						SNew(SExpanderArrow, SharedThis(this))
						.IndentAmount(10.0f)
						.ShouldDrawWires(true)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Left)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(TrackPtr->GetTrackIcon().GetSmallIcon())
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					.Padding(2.0f, 0.f)
					[
						SNew(STextBlock)
						.Text(TrackPtr->GetTrackName())
					]
				]
			]
		];
	}
};

TSharedRef<ITableRow> SDopeSheetOutliner::MakeTableRowWidget(
	FDopeSheetTrackViewModelRef          InItem,
	const TSharedRef<STableViewBase>& TableViewBase)
{
	return SNew(SDopeSheetOutlinerRow, TableViewBase, InItem);
}

void SDopeSheetOutliner::HandleGetChildren(FDopeSheetTrackViewModelRef          InItem,
                                                TArray<FDopeSheetTrackViewModelRef>& OutChildren)
{
	OutChildren.Append(InItem->Children);
}

void SDopeSheetOutliner::SetExpansionRecursive(FDopeSheetTrackViewModelRef InItem, bool bShouldBeExpanded)
{
	for (FDopeSheetTrackViewModelRef ChildModel : InItem->Children)
	{
		if (ChildModel->ObjPtr)
		{
			ChildModel->ObjPtr->bIsExpanded = bShouldBeExpanded;

			SetExpansionRecursive(ChildModel, bShouldBeExpanded);
		}
	}
}

void SDopeSheetOutliner::OnExpansionChanged(TSharedRef<FDopeSheetTrackViewModel> ItemExpanded, bool bShouldBeExpanded)
{
	if (Controller.IsValid())
	{
		Controller->OnTrackExpanded(ItemExpanded, bShouldBeExpanded);
	}

	RequestTreeRefresh();
}


void SDopeSheetOutliner::Construct(const FArguments& InArgs, TSharedPtr<FDopeSheetController> InController)
{
	check(InController)
	Controller = InController;
	Controller->OnUpdate.AddLambda([this](EDopeSheetFlags UpdatedFlags)
	{
		if (EnumHasAnyFlags(UpdatedFlags, EDopeSheetFlags::RebuildTracks | EDopeSheetFlags::VLayoutChanged))
		{
			RequestTreeRefresh();
		}
	});

	
	STreeView<FDopeSheetTrackViewModelRef>::FArguments SuperArgs;
	SuperArgs.TreeViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("PropertyTable.InViewport.ListView"));
	SuperArgs.TreeItemsSource(&Controller->RootTracks);
	SuperArgs.SelectionMode(ESelectionMode::Multi);
	SuperArgs.OnExpansionChanged(this, &SDopeSheetOutliner::OnExpansionChanged);
	SuperArgs.OnSetExpansionRecursive(this, &SDopeSheetOutliner::SetExpansionRecursive);
	SuperArgs.ExternalScrollbar(InArgs._ExternalScrollbar);
	SuperArgs.AllowOverscroll(EAllowOverscroll::No); 
	SuperArgs.OnGenerateRow(this, &SDopeSheetOutliner::MakeTableRowWidget);
	SuperArgs.OnGetChildren(this, &SDopeSheetOutliner::HandleGetChildren);
	// SuperArgs.OnSelectionChanged(
	// 	FOnMultiRigTreeSelectionChanged::CreateRaw(&Delegates, &FMultiRigTreeDelegates::HandleSelectionChanged));
	// SuperArgs.OnContextMenuOpening(Delegates.OnContextMenuOpening);
	// SuperArgs.OnMouseButtonClick(Delegates.OnMouseButtonClick);
	// SuperArgs.OnMouseButtonDoubleClick(Delegates.OnMouseButtonDoubleClick);




	STreeView<FDopeSheetTrackViewModelRef>::Construct(SuperArgs);
}

void SDopeSheetOutliner::SetFilterText(const FText& Text)
{
	RequestTreeRefresh();
};
