#include "SDopeSheetOutliner.h"

#include "DopeSheetController.h"
#include "Tracks/DopeSheetTrackBase.h"
#include "Tracks/DopeSheetTrackViewModel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Views/STreeView.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SEditableTextBox.h"


class SDopeSheetOutlinerRow : public STableRow<FDopeSheetTrackViewModelRef>
{
	SLATE_BEGIN_ARGS(SDopeSheetOutlinerRow)
		{
		}

		SLATE_EVENT(FOnClicked, OnAddSubTrackClicked)
	SLATE_END_ARGS()


	TSharedPtr<SHorizontalBox> InnerRow;

	virtual bool IsItemExpanded() const override
	{
		return true;
	}

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
				.BorderBackgroundColor(TrackPtr->GetSectionColor().Desaturate(.75f))
				.Padding(0.f)
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SAssignNew(InnerRow, SHorizontalBox)
					+ SHorizontalBox::Slot()
					.MinWidth(15.f)
					.MaxWidth(15.f)
					.FillWidth(1.f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("WhiteBrush"))
						.ColorAndOpacity(TrackPtr->GetTrackColor())
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					.AutoWidth()
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
					.AutoWidth()
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
					.Padding(4.0f, 0.f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Left)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.AutoWrapText(false)
						.WrapTextAt(300.f)
						.Text(TrackPtr->GetTrackName())
					]
				
				]
			]
		];

		if (TrackModel->ObjPtr->CanCreateSubTracks())
		{
			FSlateFontInfo SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Regular", 8);
			InnerRow->AddSlot()
					.FillWidth(1.f)
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Right)
					.Padding(4.0f, 4.f)
			[
				SNew(SButton)
				.HAlign(HAlign_Fill)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ForegroundColor(FSlateColor::UseForeground())
				.OnClicked(InArgs._OnAddSubTrackClicked)
				.ToolTipText(NSLOCTEXT("DopeSheetOutliner", "AddSubTrackButton", "Add a new sub-track"))
				.ContentPadding(2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.Plus"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					.FillWidth(1.f)
					[
						SNew(STextBlock)
						.Text(FText(NSLOCTEXT("DopeSheetOutliner", "AddSubTrackButton", "Track")))
						.Font(SmallLayoutFont)
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			];
		}
	}
};


TSharedRef<ITableRow> SDopeSheetOutliner::MakeTableRowWidget(
	FDopeSheetTrackViewModelRef       InItem,
	const TSharedRef<STableViewBase>& TableViewBase)
{
	if (InItem->ObjPtr)
	{
		SetExpansionRecursive(InItem, InItem->ObjPtr->bIsExpanded);
	}

	return SNew(SDopeSheetOutlinerRow, TableViewBase, InItem)
		.OnAddSubTrackClicked(this, &SDopeSheetOutliner::OnAddTrackClicked, InItem);
}

void SDopeSheetOutliner::HandleGetChildren(FDopeSheetTrackViewModelRef          InItem,
                                                TArray<FDopeSheetTrackViewModelRef>& OutChildren)
{
	OutChildren.Append(InItem->Children);
}

void SDopeSheetOutliner::SetExpansionRecursive(FDopeSheetTrackViewModelRef InItem, bool bShouldBeExpanded)
{
	SetItemExpansion(InItem, bShouldBeExpanded);
	
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

bool SDopeSheetOutliner::IsDeleteEnabled() const
{
	return SelectedItems.Num() > 0;
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
	SuperArgs.OnGenerateRow(this, &SDopeSheetOutliner::MakeTableRowWidget);
	SuperArgs.OnGetChildren(this, &SDopeSheetOutliner::HandleGetChildren);
	SuperArgs.ExternalScrollbar(InArgs._ExternalScrollbar);
	SuperArgs.AllowOverscroll(EAllowOverscroll::No);
	SuperArgs.OnContextMenuOpening(this, &SDopeSheetOutliner::OnContextMenuOpening);
	// SuperArgs.OnSelectionChanged(this, &SDopeSheetOutliner::OnSelectionChanged);
	// SuperArgs.OnMouseButtonClick(SDopeSheetOutliner::HandleMouseClick);
	// SuperArgs.OnMouseButtonDoubleClick(Delegates.OnMouseButtonDoubleClick);
	
	
	
	
	STreeView<FDopeSheetTrackViewModelRef>::Construct(SuperArgs);
}

FReply SDopeSheetOutliner::OnAddTrackClicked(FDopeSheetTrackViewModelRef ParentTrack)
{
	
	TArray<UClass*> TrackClasses;
	ParentTrack->ObjPtr->GetSubTrackClasses(TrackClasses);
	
	const bool   bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, nullptr);

	MenuBuilder.BeginSection("AddTrack", NSLOCTEXT("DopeSheetOutliner", "AddTrackMenuHeader", "Add New Track"));
	{
		
		// Iterate over all loaded UClasses to find subclasses of UDopeSheetTrackBase
		for (UClass* Class : TrackClasses)
		{
			if (!Class->HasAnyClassFlags(CLASS_Abstract) && Class->IsChildOf(UDopeSheetTrackBase::StaticClass()))
			{
				UDopeSheetTrackBase* TrackCDO = Class->GetDefaultObject<UDopeSheetTrackBase>();
				MenuBuilder.AddMenuEntry(
					TrackCDO->GetTrackName(),
					TrackCDO->GetTrackName(),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &SDopeSheetOutliner::OnAddTrackSelected, Class, ParentTrack))
					);

			}
		}
	}
	MenuBuilder.EndSection();


	// Show the menu anchored to the add button
	FSlateApplication::Get().PushMenu(
		AsShared(),
		FWidgetPath(),
		MenuBuilder.MakeWidget(),
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
		);

	return FReply::Handled();
}

FReply SDopeSheetOutliner::OnRemoveTrackClicked()
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "RemoveDopeSheetTracks_Transaction", "Remove DopeSheet Tracks"), !GIsTransacting);
	for (auto TrackModelRef : SelectedItems)
	{
		Controller->RemoveTrack(TrackModelRef);
	}

	return FReply::Handled();
}

void SDopeSheetOutliner::OnAddTrackSelected(UClass* TrackClass, FDopeSheetTrackViewModelRef ModelRef)
{
	// MontageTracks.Add(NewObject<TrackClass>(this));
	Controller->AddSubTrackToSelected(TrackClass, ModelRef);
}

FReply SDopeSheetOutliner::HandleMouseClick()
{
	return FReply::Handled();
}

void SDopeSheetOutliner::SetFilterText(const FText& Text)
{
	RequestTreeRefresh();
};

TSharedPtr<SWidget> SDopeSheetOutliner::OnContextMenuOpening()
{
	if (SelectedItems.IsEmpty())
	{
		// Empty-space right-click: offer adding a new root-level track
		TArray<UClass*> RootTrackClasses;
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* Class = *It;
			if (!Class->HasAnyClassFlags(CLASS_Abstract) && Class->IsChildOf(UDopeSheetTrackBase::StaticClass()))
			{
				RootTrackClasses.Add(Class);
			}
		}

		if (RootTrackClasses.IsEmpty())
		{
			return nullptr;
		}

		FMenuBuilder MenuBuilder(true, nullptr);
		MenuBuilder.BeginSection("AddTrack", NSLOCTEXT("DopeSheetOutliner", "AddTrackHeader", "Add Track"));
		{
			for (UClass* Class : RootTrackClasses)
			{
				UDopeSheetTrackBase* CDO = Class->GetDefaultObject<UDopeSheetTrackBase>();
				MenuBuilder.AddMenuEntry(
					CDO->GetTrackName(),
					CDO->GetTrackName(),
					CDO->GetTrackIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &SDopeSheetOutliner::OnAddRootTrackSelected, Class))
				);
			}
		}
		MenuBuilder.EndSection();
		return MenuBuilder.MakeWidget();
	}

	FMenuBuilder MenuBuilder(true, nullptr);

	// ---------------------------------------------------------------
	// Collections section
	// ---------------------------------------------------------------
	MenuBuilder.BeginSection("Collections", NSLOCTEXT("DopeSheetOutliner", "CollectionsSectionHeader", "Move to Collection"));
	{
		// Existing collections
		TSet<FName> ExistingCollections = Controller->GetAllCollectionNames();
		for (const FName& Name : ExistingCollections)
		{
			MenuBuilder.AddMenuEntry(
				FText::FromName(Name),
				NSLOCTEXT("DopeSheetOutliner", "MoveToCollectionTip", "Move selected tracks to this collection"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SDopeSheetOutliner::AssignSelectedTracksToCollection, Name))
			);
		}

		// Inline "New Collection" text entry
		TSharedRef<SEditableTextBox> NameBox =
			SNew(SEditableTextBox)
			.HintText(NSLOCTEXT("DopeSheetOutliner", "NewCollectionHint", "New collection name..."))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type CommitType)
			{
				if (CommitType == ETextCommit::OnEnter && !Text.IsEmpty())
				{
					AssignSelectedTracksToCollection(FName(*Text.ToString()));
					FSlateApplication::Get().DismissAllMenus();
				}
			});

		MenuBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(4.f, 2.f)
			.FillWidth(1.f)
			[
				NameBox
			],
			NSLOCTEXT("DopeSheetOutliner", "NewCollectionLabel", "New Collection"),
			/*bNoIndent=*/false
		);
	}
	MenuBuilder.EndSection();

	// ---------------------------------------------------------------
	// Track-specific actions (deduplicated by class)
	// ---------------------------------------------------------------
	bool bAllSelectedDeletable = true;
	TSet<UClass*> VisitedClasses;
	bool bAddedTrackSection = false;
	for (const FDopeSheetTrackViewModelRef& TrackModel : SelectedItems)
	{
		UDopeSheetTrackBase* Track = TrackModel->ObjPtr;
		if (!Track) { continue; }

		if (!Track->bAllowDelete)
		{
			bAllSelectedDeletable = false;
		}

		if (VisitedClasses.Contains(Track->GetClass())) { continue; }
		VisitedClasses.Add(Track->GetClass());

		if (!bAddedTrackSection)
		{
			MenuBuilder.BeginSection("TrackActions", NSLOCTEXT("DopeSheetOutliner", "TrackActionsSectionHeader", "Track"));
			bAddedTrackSection = true;
		}
		Track->BuildContextMenuActions(MenuBuilder);
	}

	// "Delete Track" — only shown when every selected track permits deletion.
	if (bAllSelectedDeletable)
	{
		if (!bAddedTrackSection)
		{
			MenuBuilder.BeginSection("TrackActions", NSLOCTEXT("DopeSheetOutliner", "TrackActionsSectionHeader", "Track"));
			bAddedTrackSection = true;
		}
		MenuBuilder.AddMenuEntry(
			NSLOCTEXT("DopeSheetOutliner", "DeleteTrack", "Delete Track"),
			NSLOCTEXT("DopeSheetOutliner", "DeleteTrackTip", "Remove the selected track(s) from this node"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
			FUIAction(FExecuteAction::CreateLambda([this]() { OnRemoveTrackClicked(); }))
		);
	}

	if (bAddedTrackSection)
	{
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

void SDopeSheetOutliner::AssignSelectedTracksToCollection(FName CollectionName)
{
	TArray<UDopeSheetTrackBase*> Tracks;
	for (const FDopeSheetTrackViewModelRef& Item : SelectedItems)
	{
		if (Item->ObjPtr)
		{
			Tracks.Add(Item->ObjPtr);
		}
	}
	Controller->MoveTracksToCollection(Tracks, CollectionName);
}

void SDopeSheetOutliner::OnAddRootTrackSelected(UClass* TrackClass)
{
	if (Controller.IsValid())
	{
		Controller->AddTrackToSelected(TrackClass);
	}
}
