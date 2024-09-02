// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
struct FSharedPoseHandle;
/**
 * 
 */
class MONTAGEGRAPHEDITOR_API SControlRigSharedPoseDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SControlRigSharedPoseDialog)
		{
		}

	SLATE_END_ARGS()

	TSharedRef<ITableRow> CreateHandleRow(TSharedPtr<FSharedPoseHandle> InTagHandle,
	                                      const TSharedRef<STableViewBase>& InOwnerTable);

	void HandleSelectedPoseChanged(TSharedPtr<FSharedPoseHandle> NewSelection, ESelectInfo::Type Arg);
	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	TArray<TSharedPtr<FSharedPoseHandle>> Handles;
	
	TSharedPtr<FSharedPoseHandle> SelectedHandle;
	
	FString NewHandleString;
	
};
