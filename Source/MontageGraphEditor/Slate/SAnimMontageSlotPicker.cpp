// Copyright Drop Games Inc.


#include "SAnimMontageSlotPicker.h"

#include "SlateOptMacros.h"
#include "Widgets/Input/STextComboBox.h"

#define LOCTEXT_NAMESPACE "SAnimMontageSlotPicker"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAnimMontageSlotPicker::Construct(const FArguments& InArgs, UAnimSequenceBase* InSequence, FName InitialSlotName)
{
	OnSlotChanged = InArgs._OnSlotChanged;
	
	
	Skeleton = InSequence->GetSkeleton();
	for (auto Slot : Skeleton->GetSlotGroups())
	{
		SlotNameList.Append(Slot.SlotNames);

		for (auto Name : Slot.SlotNames)
		{
			SlotNameComboListItems.Add(MakeShareable(new FString(Name.ToString())));
		}
	}

	const int32               FoundIndex       = FMath::Max(SlotNameList.Find(InitialSlotName), 0);
	const TSharedPtr<FString> InitialSelection = SlotNameComboListItems[FoundIndex];

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(SlotNameComboBox, STextComboBox)
			.OptionsSource(&SlotNameComboListItems)
			.OnSelectionChanged(this, &SAnimMontageSlotPicker::OnSlotNameChanged)
			.OnComboBoxOpening(this, &SAnimMontageSlotPicker::OnSlotListOpening)
			.InitiallySelectedItem(InitialSelection)
			.ContentPadding(2.f)
			.ToolTipText(FText::FromString(*InitialSelection))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("AnimSlotNode_DetailPanelManageButtonLabel", "Anim Slot Manager"))
			.ToolTipText(LOCTEXT("AnimSlotNode_DetailPanelManageButtonToolTipText",
			                     "Open Anim Slot Manager to edit Slots and Groups."))
			.OnClicked(this, &SAnimMontageSlotPicker::OnOpenAnimSlotManager)
			.Content()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("MeshPaint.FindInCB"))
			]
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAnimMontageSlotPicker::RefreshComboLists(bool bOnlyRefreshIfDifferent /*= false*/)
{
	// SlotNodeNamePropertyHandle->GetValue(SlotNameComboSelectedName);

	// Make sure slot node exists in our skeleton.
	Skeleton->RegisterSlotNode(SlotNameComboSelectedName);

	// Refresh Slot Names
	{
		TArray<TSharedPtr<FString>> NewSlotNameComboListItems;
		TArray<FName>               NewSlotNameList;
		bool                        bIsSlotNameListDifferent = false;

		const TArray<FAnimSlotGroup>& SlotGroups = Skeleton->GetSlotGroups();
		for (auto SlotGroup : SlotGroups)
		{
			int32 Index = 0;
			for (auto SlotName : SlotGroup.SlotNames)
			{
				NewSlotNameList.Add(SlotName);

				FString ComboItemString = FString::Printf(TEXT("%s.%s"), *SlotGroup.GroupName.ToString(),
				                                          *SlotName.ToString());
				NewSlotNameComboListItems.Add(MakeShareable(new FString(ComboItemString)));

				bIsSlotNameListDifferent = bIsSlotNameListDifferent || (!SlotNameComboListItems.IsValidIndex(Index) || (
					SlotNameComboListItems[Index] != NewSlotNameComboListItems[Index]));
				Index++;
			}
		}

		// Refresh if needed
		if (bIsSlotNameListDifferent || !bOnlyRefreshIfDifferent || (NewSlotNameComboListItems.Num() == 0))
		{
			SlotNameComboListItems = NewSlotNameComboListItems;
			SlotNameList           = NewSlotNameList;

			if (SlotNameComboBox.IsValid())
			{
				if (Skeleton->ContainsSlotName(SlotNameComboSelectedName))
				{
					int32               FoundIndex = SlotNameList.Find(SlotNameComboSelectedName);
					TSharedPtr<FString> ComboItem  = SlotNameComboListItems[FoundIndex];

					SlotNameComboBox->SetSelectedItem(ComboItem);
					SlotNameComboBox->SetToolTipText(FText::FromString(*ComboItem));
				}
				SlotNameComboBox->RefreshOptions();
			}
		}
	}
}

////////////////////////////////////////////////////////////////

void SAnimMontageSlotPicker::OnSlotNameChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	// if it's set from code, we did that on purpose
	if (SelectInfo != ESelectInfo::Direct)
	{
		int32 ItemIndex = SlotNameComboListItems.Find(NewSelection);
		if (ItemIndex != INDEX_NONE)
		{
			SlotNameComboSelectedName = SlotNameList[ItemIndex];
			if (SlotNameComboBox.IsValid())
			{
				SlotNameComboBox->SetToolTipText(FText::FromString(*NewSelection));
			}

			if (Skeleton->ContainsSlotName(SlotNameComboSelectedName))
			{
				// trigger transaction 
				//const FScopedTransaction Transaction(LOCTEXT("ChangeSlotNodeName", "Change Collision Profile"));
				// set profile set up
				// ensure(SlotNodeNamePropertyHandle->SetValue(SlotNameComboSelectedName.ToString()) == FPropertyAccess::Result::Success);
				
				OnSlotChanged.ExecuteIfBound(SlotNameComboSelectedName);
			}
		}
	}
}

void SAnimMontageSlotPicker::OnSlotListOpening()
{
	// Refresh Slot Names, in case we used the Anim Slot Manager to make changes.
	RefreshComboLists(true);
}

FReply SAnimMontageSlotPicker::OnOpenAnimSlotManager()
{
	// OnInvokeTab.ExecuteIfBound(FPersonaTabs::SkeletonSlotNamesID);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
