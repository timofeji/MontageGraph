// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnSlotChanged, FName);
/**
 * 
 */
class MONTAGEGRAPHEDITOR_API SAnimMontageSlotPicker : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS(SAnimMontageSlotPicker)
		{
		}
		SLATE_EVENT(FOnSlotChanged, OnSlotChanged)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UAnimSequenceBase* InSequence, FName InitialSlotName);
	
	

	// slot node names


	FReply OnOpenAnimSlotManager();
	void   OnSlotListOpening();
	void   OnSlotNameChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void   RefreshComboLists(bool bOnlyRefreshIfDifferent /*= false*/);

private:
	TSharedPtr<class STextComboBox> SlotNameComboBox;
	TArray<TSharedPtr<FString>>     SlotNameComboListItems;
	TArray<FName>                   SlotNameList;
	FName                           SlotNameComboSelectedName;

	USkeleton* Skeleton;
	
	FOnSlotChanged OnSlotChanged;
};
