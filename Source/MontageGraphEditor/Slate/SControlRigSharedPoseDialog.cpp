// Fill out your copyright notice in the Description page of Project Settings.


#include "SControlRigSharedPoseDialog.h"

#include "DetailLayoutBuilder.h"
#include "MontageGraphEditorStyles.h"
#include "SlateOptMacros.h"
#include "Sequencer/MontageGraphSharedPoseTypes.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

#define LOCTEXT_NAMESPACE "ControlRigSharedPoseDialog"

TSharedRef<ITableRow> SControlRigSharedPoseDialog::CreateHandleRow(TSharedPtr<FSharedPoseHandle> InTagHandle,
                                                                   const TSharedRef<STableViewBase>& InOwnerTable)
{
	check(InTagHandle.IsValid());
	FSharedPoseHandle TagHandle = *InTagHandle;

	// Disable Selecting Items as this is handle already by SAvaTagHandleEntry
	class SAvaTagRow : public STableRow<TSharedPtr<FSharedPoseHandle>>
	{
	public:
		//~ Begin STableRow
		virtual FReply OnMouseButtonDown(const FGeometry&, const FPointerEvent&) override
		{
			return FReply::Unhandled();
		}

		virtual FReply OnMouseButtonUp(const FGeometry&, const FPointerEvent&) override { return FReply::Unhandled(); }
		//~ End STableRow
	};

	return SNew(SAvaTagRow, InOwnerTable)
    		.Style(&FMontageGraphEditorStyles::Get().GetWidgetStyle<FTableRowStyle>("TagListView.Row"))
    		.Padding(FMargin(0, 2))
	[
		SNew(STextBlock)
			.Text(FText::FromString(TagHandle.PoseName))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Visibility(EVisibility::SelfHitTestInvisible)
// 	// SNew(SAvaTagHandleEntry, TagHandle)
 //  //   			.IsSelected(this, &SAvaTagPicker::IsTagHandleSelected, TagHandle)
 //  //   			.OnSelectionChanged(this, &SAvaTagPicker::OnTagHandleSelectionChanged)
 //  //   			.ShowCheckBox(TagCustomizer->AllowMultipleTags())
	];
}

void SControlRigSharedPoseDialog::Construct(const FArguments& InArgs)
{
	Handles.Add(MakeShared<FSharedPoseHandle>("Combo1_End"));
	Handles.Add(MakeShared<FSharedPoseHandle>("Combo2_End"));
	Handles.Add(MakeShared<FSharedPoseHandle>("Combo3_End"));

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		  .FillHeight(1.f)
		  .HAlign(HAlign_Fill)
		  .Padding(5.f)
		[
			SNew(SListView<TSharedPtr<FSharedPoseHandle>>)
         		.ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("SimpleListView"))
         		.OnGenerateRow(this, &SControlRigSharedPoseDialog::CreateHandleRow)
         		.SelectionMode(ESelectionMode::Single)
         		.ListItemsSource(&Handles)
         		.ItemHeight(24.0f)
         		.IsFocusable(false)
         		.HandleGamepadEvents(false)
         		.HandleSpacebarSelection(false)
         		.HandleDirectionalNavigation(false)
		]
		+ SVerticalBox::Slot()
		  .AutoHeight()
		  .HAlign(HAlign_Right)
		  .Padding(5.f)
		[
			SNew(SButton)
			.ContentPadding(FMargin(10, 5))
			.Text(LOCTEXT("CreateControlAssetRig", "Create New Shared Pose"))
			// .OnClicked(this, &SCreateControlAssetRigDialog::OnCreateControlAssetRig)
		]

	];
}

#undef LOCTEXT_NAMESPACE
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
