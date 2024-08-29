
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "MontageGraphEdGraph.generated.h"

class UMontageGraph;
class FMontageGraphDebugger;
class UMontageGraphEdNode;
class UMontageGraphEdNodeSelector;
class UMontageGraphNode;
class UHBMontageGraphNodeAnim;
class UHBMontageGraphAbility;
class UMontageGraphEdge;
class UMontageGraphEdNodeEdge;
class UMontageGraphEdNode;
class UMontageGraphEdNodeEntry;

UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageGraphEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	/** Entry node within the state machine */
	UPROPERTY()
	TArray<UMontageGraphEdNode*> EntryNodes;

	UPROPERTY(Transient)
	TMap<UMontageGraphNode*, UMontageGraphEdNode*> NodeMap;

	UPROPERTY(Transient)
	TMap<UMontageGraphEdge*, UMontageGraphEdNodeEdge*> EdgeMap;

	/** Shared ref to asset editor, namely to access debugger info and debugged node */
	TSharedPtr<FMontageGraphDebugger> Debugger;

	virtual void RebuildGraph();

	UMontageGraph* GetHBMontageGraphModel() const;
	void            RebuildGraphForSelector(UMontageGraph* HbMontageGraph, UMontageGraphEdNodeSelector* SelectorNode);
	void            RebuildGraphForEdge(UMontageGraph* OwningGraph, UMontageGraphEdNodeEdge* EdGraphEdge);
	void            RebuildGraphForNode(UMontageGraph* OwningGraph, UMontageGraphEdNode* Node);
	void            RebuildGraphForEntry(UMontageGraph* OwningGraph, UMontageGraphEdNodeEntry* NodeEntry);

	/** Goes through each nodes and run a validation pass */
	void ValidateNodes(FCompilerResultsLog* LogResults);

	//~ UObject interface
	virtual bool Modify(bool bAlwaysMarkDirty) override;
	virtual void PostEditUndo() override;
	//~ End UObject interface

	/** Returns all graph nodes that are of node type (not edges) */
	TArray<UMontageGraphEdNode*> GetAllNodes() const;

	/** Re-organize graph nodes automatically */
	void AutoArrange(bool bVertical);

protected:
	void Clear();

	// TODO: Is it needed anymore ?
	void SortNodes(UMontageGraphNode* RootNode);
};
