﻿// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphNode.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MontageGraph.generated.h"

class UInputAction;
	
/**
 * 
 */
UCLASS()
class MONTAGEGRAPH_API UMontageGraph : public UObject
{
	GENERATED_BODY()


public:
	UMontageGraph();

	/**
	 * Clear all nodes in the graph 
	 */
	void ClearGraph();
	
	//~ IInterface_PreviewMeshProvider
	// virtual void SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty) override;
	// virtual USkeletalMesh* GetPreviewMesh() const override;
	//~ End IInterface_PreviewMeshProvider

	/*We're only storing a spatial representation in the editor,
	 *OnSave we'll convert this to a serialized representation*/

	
	/** Represents the top level entry into various action chains*/
	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TMap<FGameplayTag,UMontageGraphNode*> EntryNodes;

	/** Holds all HBActionnodes defined for this graph */
	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TArray<UMontageGraphNode*> AllNodes;

	
	int MaxDepth;

#if WITH_EDITORONLY_DATA
	/** Internal Node Class type used to know which classes to consider when drawing out context menu in graphs */
	TSubclassOf<UMontageGraphNode> NodeType;

	/** Not used currently, but might be if we allow customization of edge classes */
	TSubclassOf<UMontageGraphEdge> EdgeType;

	
	UPROPERTY()
	UEdGraph* EditorGraph;
	//
	// /** Last selected app mode, stored here to persist across re-opening and restart of the editor */
	// UPROPERTY(VisibleDefaultsOnly, Category = "Montage Graph", meta=(HideInMontageGraphDetailsPanel))
	// EMontageGraphSelectedAppMode SelectedAppMode = EMontageGraphSelectedAppMode::Persona;

private:
	/** The default skeletal mesh to use when previewing this asset */
	UPROPERTY(duplicatetransient, AssetRegistrySearchable)
	TSoftObjectPtr<class USkeletalMesh> PreviewSkeletalMesh;
#endif
};