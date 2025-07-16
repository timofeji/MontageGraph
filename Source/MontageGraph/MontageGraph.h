// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "GameplayPrediction.h"
#include "Nodes/MGNode.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MontageGraph.generated.h"

class UCollisionTracer;
/**
 * 
 */
UCLASS()
class MONTAGEGRAPH_API UMontageGraph : public UObject, public IInterface_PreviewMeshProvider
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
	TMap<FGameplayTag,UMGNode*> RootNodes;
	

	/** Holds all HBActionnodes defined for this graph */
	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TArray<UMGNode*> AllNodes;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Montage Graph")
	TArray<TSubclassOf<class UGameplayEffect>> GameplayEffects;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Montage Graph")
	TArray<UAnimMontage*> Montages;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Montage Graph")
	TArray<UCollisionTracer*> CollisionTracers;
	

	

	/** IPreviewMeshProviderInterface interface */
	virtual void SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty = true);
	virtual USkeletalMesh* GetPreviewMesh() const;

	/** The default skeletal mesh to use when previewing this asset */
	UPROPERTY(duplicatetransient, AssetRegistrySearchable)
	TSoftObjectPtr<class USkeletalMesh> PreviewSkeletalMesh;

#if WITH_EDITORONLY_DATA
	/** Internal Node Class type used to know which classes to consider when drawing out context menu in graphs */
	TSubclassOf<UMGNode> NodeType;

	/** Not used currently, but might be if we allow customization of edge classes */
	TSubclassOf<UMGEdge> EdgeType;

	UPROPERTY()
	UEdGraph* EditorGraph;
#endif

};
