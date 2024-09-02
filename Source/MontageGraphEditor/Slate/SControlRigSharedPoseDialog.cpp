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
	class SSharedPoseHandleRow : public STableRow<TSharedPtr<FSharedPoseHandle>>
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

	return SNew(SSharedPoseHandleRow, InOwnerTable)
    		.Style(&FMontageGraphEditorStyles::Get().GetWidgetStyle<FTableRowStyle>("TagListView.Row"))
    		.Padding(FMargin(0, 2))
	[
		SNew(STextBlock)
			.Text(FText::FromString(TagHandle.PoseName))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Visibility(EVisibility::SelfHitTestInvisible)
			.ColorAndOpacity(FSlateColor::UseForeground())

		// 	// SNew(SAvaTagHandleEntry, TagHandle)
		//  //   			.IsSelected(this, &SAvaTagPicker::IsTagHandleSelected, TagHandle)
		//  //   			.OnSelectionChanged(this, &SAvaTagPicker::OnTagHandleSelectionChanged)
		//  //   			.ShowCheckBox(TagCustomizer->AllowMultipleTags())
	];
}

void SControlRigSharedPoseDialog::HandleSelectedPoseChanged(TSharedPtr<FSharedPoseHandle> NewSelection,
                                                            ESelectInfo::Type Arg)
{
	SelectedHandle = NewSelection;
}

void SControlRigSharedPoseDialog::Construct(const FArguments& InArgs)
{
	Handles.Add(MakeShared<FSharedPoseHandle>("Combo1_End"));
	Handles.Add(MakeShared<FSharedPoseHandle>("Combo2_End"));
	Handles.Add(MakeShared<FSharedPoseHandle>("Combo3_End"));

	TSharedRef<SListView<TSharedPtr<FSharedPoseHandle>>> HandlesList =
		SNew(SListView<TSharedPtr<FSharedPoseHandle>>)
    				.ListViewStyle(FMontageGraphEditorStyles::Get(), "SharedPoseList")
             		.OnGenerateRow(this, &SControlRigSharedPoseDialog::CreateHandleRow)
    				.OnSelectionChanged(this, &SControlRigSharedPoseDialog::HandleSelectedPoseChanged)
             		.SelectionMode(ESelectionMode::Single)
             		.ListItemsSource(&Handles)
             		.ItemHeight(24.0f)
             		.IsFocusable(false)
             		.HandleGamepadEvents(false)
             		.HandleSpacebarSelection(false)
             		.HandleDirectionalNavigation(false);


	ChildSlot
	[
		SNew(SVerticalBox)
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
				.OnClicked_Lambda([this, HandlesList]()
				             {
					             auto NewHandle = MakeShared<FSharedPoseHandle>(NewHandleString);
					             Handles.Add(NewHandle);
					             HandlesList->SetSelection(NewHandle);
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
		  .HAlign(HAlign_Right)
		  .Padding(5.f)
		[
			SNew(SButton)
			.ContentPadding(FMargin(10, 5))
			.Text(LOCTEXT("LinkPoseBtn", "Link Current Frame to Shared Pose"))
		]
	];
}

#undef LOCTEXT_NAMESPACE
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
