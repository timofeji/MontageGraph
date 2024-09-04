// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphNode.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MontageGraph.generated.h"

struct FSharedPoseHandle;
class UControlRig;
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

	UPROPERTY(EditAnywhere, Category = "Montage Graph Defaults")
	TSubclassOf<UControlRig> ControlRigClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage Graph Defaults")
	FString GraphOutputBaseDir = "MontageGraphAssets";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage Graph Defaults")
	FString GraphOutputPrefix;


private:
	/** The default skeletal mesh to use when previewing this asset */
	UPROPERTY(duplicatetransient, AssetRegistrySearchable)
	TSoftObjectPtr<class USkeletalMesh> PreviewSkeletalMesh;
#endif
};
