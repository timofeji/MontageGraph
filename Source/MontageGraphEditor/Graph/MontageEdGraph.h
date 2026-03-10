#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "MontageGraph/Tracers/MontageCollisionTracer.h"
#include "MontageEdGraph.generated.h"

struct FGameplayTagContainer;
struct FGameplayTag;

class UMGNode_Montage;
class UMGEdNode_Montage;
class UMontageGraph;
class FMontageGraphDebugger;
class UMGEdNode;
class UMGEdNode_Selector;
class UMGNode;
class UMontageGraphNodeAnim;
class UMGEdge;
class UMGEdNode_Edge;
class UMGEdNode;
class UMGEdNode_Entry;

UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	/** Entry node within the state machine */
	UPROPERTY()
	TArray<UMGEdNode*> EntryNodes;

	UPROPERTY(Transient)
	TMap<UMGNode*, UMGEdNode*> NodeMap;

	UPROPERTY(Transient)
	TMap<UMGEdge*, UMGEdNode_Edge*> EdgeMap;

	/** Shared ref to asset editor, namely to access debugger info and debugged node */
	TSharedPtr<FMontageGraphDebugger> Debugger;

	void MapGraphAggregateTags(UMontageGraph* MontageGraph);
	void MapSubGraph(UMGNode* Root, FGameplayTagContainer TraversedTags);

	
	virtual void RebuildStaleMontages();

	virtual void RebuildRuntimeGraph();


	UMontageGraph* GetMontageGraphModel() const;
	void            RebuildGraphForSelector(UMontageGraph* MontageGraph, UMGEdNode_Selector* EdNode);
	void            RebuildGraphForEdge(UMontageGraph* OwningGraph, UMGEdNode_Edge* EdGraphEdge);


	
	int           GetDepth(UMGNode* Root);
	int           GetBreadth(UMGNode* Root);

	/**
	 * Maps nodes for a specific root node.
	 ***/
	void RebuildMontageTreeForEntryBind(UMGEdNode* RootNode, UMontageGraph* OwningGraph);
	
	void RebuildNodeTree(UMGEdNode* RootNode, UMontageGraph* OwningGraph, TArray<UMGEdNode_Montage*>& MontageEdNodes);
	
	void MapLinkedMontages(UMGEdNode_Montage* RootNode, TArray<UMGNode_Montage*>& OutMontages);
	
	

	/** Goes through each nodes and run a validation pass */
	void ValidateNodes(FCompilerResultsLog* LogResults);

	//~ UObject interface
	virtual bool Modify(bool bAlwaysMarkDirty) override;
	virtual void PostEditUndo() override;
	//~ End UObject interface

	/** Returns all graph nodes that are of node type (not edges) */
	TArray<UMGEdNode*> GetAllNodes() const;

	/** Re-organize graph nodes automatically */
	void AutoArrange(bool bVertical);
	
	void GetEditingType();

protected:
	void Clear();

	// TODO: Is it needed anymore ?
	void SortNodes(UMGNode* RootNode);

};
