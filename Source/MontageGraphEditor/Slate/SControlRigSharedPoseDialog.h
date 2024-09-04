// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MontageGraph/MontageGraph.h"
#include "Widgets/SCompoundWidget.h"
class ISequencer;
class UMontageGraphEdGraph;
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

	void HandleSelectedPoseChanged(TSharedPtr<FSharedPoseHandle> InFilter, ESelectInfo::Type SelectInfo);

	FReply LinkSelectedSharedPoseToFrame();
	void GraphSelected(const FAssetData& AssetData, TSharedRef<SListView<TSharedPtr<FSharedPoseHandle>>> List);
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs,  TSharedPtr<ISequencer> SequencerRef);
	
	TArray<TSharedPtr<FSharedPoseHandle>> Handles;
	
	TSharedPtr<FSharedPoseHandle> SelectedHandle;
	
	FString NewHandleString;
	
	TSharedPtr<IPropertyHandle> SourceMontageGraphPropertyHandle;
	
	UMontageGraphEdGraph* SelectedGraph;
	TSharedPtr<ISequencer>  Sequencer;
};
