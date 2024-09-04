// Fill out your copyright notice in the Description page of Project Settings.


#include "SControlRigSharedPoseDialog.h"

#include "DetailLayoutBuilder.h"
#include "MontageGraphEditorStyles.h"
#include "PropertyCustomizationHelpers.h"
#include "SlateOptMacros.h"
#include "Graph/MontageGraphEdGraph.h"
#include "MontageGraph/MontageGraph.h"
#include "Sequencer/MontageGraphSharedPoseTypes.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

#define LOCTEXT_NAMESPACE "ControlRigSharedPoseDialog"


// Disable Selecting Items as this is handle already by SAvaTagHandleEntry
class SSharedPoseHandleRow : public STableRow<TSharedPtr<FSharedPoseHandle>>
{
public:
	TSharedPtr<FSharedPoseHandle> PoseHandle;

	// void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView,
	//                TSharedPtr<FSharedPoseHandle>& InHandle)
	// {
	// 	PoseHandle = InHandle;
	// 	SetOwnerTableView(InOwnerTableView);
	// 	ChildSlot
	// 
	//
	// 	//
	// 	// TSharedRef<SWrapBox> WrapBox = SNew(SWrapBox)
	// 	// 	.UseAllottedWidth(true);
	// 	//
	// 	// TAttribute<bool> FilterEnabled = TAttribute<bool>::Create(
	// 	// 	TAttribute<bool>::FGetter::CreateSP(PoseHandle.Get(), &IFilterObject::IsFilterEnabled));
	// 	// WrapBox->SetEnabled(FilterEnabled);
	// 	//
	// 	//
	// 	// STableRow<TSharedPtr<IFilterObject>>::Construct(
	// 	// 	STableRow<TSharedPtr<IFilterObject>>::FArguments()
	// 	// 	.Padding(FMargin(0.f, 2.f, 0.f, 2.f))
	// 	// 	.Content()
	// 	// 	[
	// 	// 		WrapBox
	// 	// 	], InOwnerTableView);
	// 	//
	// 	// TAttribute<FText> ToolTipText = TAttribute<FText>::Create(
	// 	// 	TAttribute<FText>::FGetter::CreateSP(PoseHandle.Get(), &IFilterObject::GetToolTipText));
	// 	// SetToolTipText(ToolTipText);
	// }


	friend class SControlRigSharedPoseDialog;
};

void SControlRigSharedPoseDialog::HandleSelectedPoseChanged(TSharedPtr<FSharedPoseHandle> InHandle,
                                                            ESelectInfo::Type SelectInfo)
{
	SelectedHandle = InHandle;
}

FReply SControlRigSharedPoseDialog::LinkSelectedSharedPoseToFrame()
{
	return FReply::Handled();
}

void SControlRigSharedPoseDialog::GraphSelected(const FAssetData& InAssetData,
                                                TSharedRef<SListView<TSharedPtr<FSharedPoseHandle>>> List)
{
	UMontageGraph* Graph = Cast<UMontageGraph>(InAssetData.GetAsset());
	if (!Graph)
	{
		return;
	}

	UMontageGraphEdGraph* EdGraph = Cast<UMontageGraphEdGraph>(Graph->EditorGraph);
	if (!EdGraph)
	{
		return;
	}
	SelectedGraph = EdGraph;

	Handles.Empty();
	for (FSharedPoseHandle PoseHandle : SelectedGraph->SharedPoses)
	{
		Handles.Add(MakeShared<FSharedPoseHandle>(PoseHandle));
	}

	List->RequestListRefresh();

	
}

void SControlRigSharedPoseDialog::Construct(const FArguments& InArgs, TSharedPtr<ISequencer> SequencerRef)
{
	TSharedRef<SListView<TSharedPtr<FSharedPoseHandle>>> HandlesList =
		SNew(SListView<TSharedPtr<FSharedPoseHandle>>)
    				.ListViewStyle(FMontageGraphEditorStyles::Get(), "SharedPoseList")
             		.SelectionMode(ESelectionMode::Single)
             		.ListItemsSource(&Handles)
             		.ItemHeight(24.0f)
             		.IsFocusable(false)
             		.HandleGamepadEvents(false)
             		.HandleSpacebarSelection(false)
             		.HandleDirectionalNavigation(false)
					.OnSelectionChanged(this, &SControlRigSharedPoseDialog::HandleSelectedPoseChanged)
					.OnGenerateRow_Lambda([](TSharedPtr<FSharedPoseHandle> InTagHandle,
					                         const TSharedRef<STableViewBase>& InOwnerTable)
		                                              {
			                                              return SNew(SSharedPoseHandleRow, InOwnerTable)
				                                              .ShowSelection(true)
				                                              .Style(
					                                              &FMontageGraphEditorStyles::Get().GetWidgetStyle<
						                                              FTableRowStyle>("TagListView.Row"))
				                                              .Padding(FMargin(0, 2))
				                                              [
					                                              SNew(SHorizontalBox)
					                                              + SHorizontalBox::Slot()
					                                                .FillWidth(1.f)
					                                                .VAlign(VAlign_Fill)
					                                              [
						                                              SNew(STextBlock)
						                                              .Text(FText::FromString(
							                                              InTagHandle.Get()->PoseName))
						                                              .Font(IDetailLayoutBuilder::GetDetailFont())
						                                              .Visibility(EVisibility::SelfHitTestInvisible)
						                                              .ColorAndOpacity(FSlateColor::UseForeground())
					                                              ]
					                                              + SHorizontalBox::Slot()
					                                                .AutoWidth()
					                                                .HAlign(HAlign_Right)
					                                              [
						                                              SNew(SButton)
						                                              .Text(LOCTEXT(
							                                              "DeleteControlAssetRig",
							                                              "Delete Shared Pose Handle"))
						                                              .ButtonStyle(
							                                              FAppStyle::Get(), "FlatButton.Danger")
						                                              .Content()
						                                              [
							                                              SNew(SImage)
							                                              .Image(FAppStyle::GetBrush(
								                                              TEXT("LayerStats.Item.ClearButton")))
						                                              ]
					                                              ]

				                                              ];
		                                              });


	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .HAlign(HAlign_Fill)
		  .Padding(5.f)
		[
			SNew(SObjectPropertyEntryBox)
				.DisplayUseSelected(true)
				.AllowClear(true)
				.AllowedClass(UMontageGraph::StaticClass())
				.OnObjectChanged(this, &SControlRigSharedPoseDialog::GraphSelected, HandlesList)
		]
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .HAlign(HAlign_Fill)
		  .Padding(5.f)
		[

			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SEditableTextBox)
				.IsEnabled_Lambda([this]()
				                      {
					                      return IsValid(SelectedGraph);
				                      })
				.HintText(LOCTEXT("NewHandleText", "New SharedPose Name"))
				.Text_Lambda([this]() { return FText::FromString(NewHandleString); })
				.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type CommitType)
				                      {
					                      NewHandleString = NewText.ToString();
				                      })
			]
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .Padding(FMargin(2.f, 0.0f, 0.0f, 0.0f))
			[
				SNew(SButton)
				.ContentPadding(FMargin(10, 5))
				.Text(LOCTEXT("CreateControlAssetRig", "Create New Shared Pose"))
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
				.OnClicked_Lambda([this, HandlesList ]()
				             {
					             FSharedPoseHandle NewHandle;
					             NewHandle.PoseName = NewHandleString;
					             SelectedGraph->SharedPoses.Add(NewHandle);
					             Handles.Add(MakeShared<FSharedPoseHandle>(NewHandle));
					             HandlesList->RequestListRefresh();

					             return FReply::Handled();
				             })
				.Content()
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
				]
			]

		]
		+ SVerticalBox::Slot()
		  .FillHeight(1.f)
		  .HAlign(HAlign_Fill)
		  .Padding(5.f)
		[
			HandlesList
		]
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .HAlign(HAlign_Center)
		  .Padding(5.f)
		[
			SNew(SButton)
			.ContentPadding(FMargin(10, 5))
			.Text(LOCTEXT("LinkPoseBtn", "Link Current Frame to Shared Pose"))
			.OnClicked(this, &SControlRigSharedPoseDialog::LinkSelectedSharedPoseToFrame)
		]
	];
}

#undef LOCTEXT_NAMESPACE
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
