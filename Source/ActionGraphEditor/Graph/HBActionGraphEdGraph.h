
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "HBActionGraphEdGraph.generated.h"

class UActionGraph;
class FActionGraphDebugger;
class UActionGraphEdNode;
class UActionGraphEdNodeConduit;
class UActionGraphNode;
class UHBActionGraphNodeAnim;
class UHBActionGraphAbility;
class UActionGraphEdge;
class UActionGraphEdNodeEdge;
class UActionGraphEdNode;
class UActionGraphEdNodeEntry;

UCLASS()
class ACTIONGRAPHEDITOR_API UHBActionGraphEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	/** Entry node within the state machine */
	UPROPERTY()
	TArray<UActionGraphEdNode*> EntryNodes;

	UPROPERTY(Transient)
	TMap<UActionGraphNode*, UActionGraphEdNode*> NodeMap;

	UPROPERTY(Transient)
	TMap<UActionGraphEdge*, UActionGraphEdNodeEdge*> EdgeMap;

	/** Shared ref to asset editor, namely to access debugger info and debugged node */
	TSharedPtr<FActionGraphDebugger> Debugger;

	virtual void RebuildGraph();

	UActionGraph* GetHBActionGraphModel() const;
	void            RebuildGraphForConduit(UActionGraph* HbActionGraph, UActionGraphEdNodeConduit* ConduitNode);
	void            RebuildGraphForEdge(UActionGraph* OwningGraph, UActionGraphEdNodeEdge* EdGraphEdge);
	void            RebuildGraphForNode(UActionGraph* OwningGraph, UActionGraphEdNode* Node);
	void            RebuildGraphForEntry(UActionGraph* OwningGraph, UActionGraphEdNodeEntry* NodeEntry);

	/** Goes through each nodes and run a validation pass */
	void ValidateNodes(FCompilerResultsLog* LogResults);

	//~ UObject interface
	virtual bool Modify(bool bAlwaysMarkDirty) override;
	virtual void PostEditUndo() override;
	//~ End UObject interface

	/** Returns all graph nodes that are of node type (not edges) */
	TArray<UActionGraphEdNode*> GetAllNodes() const;

	/** Re-organize graph nodes automatically */
	void AutoArrange(bool bVertical);

protected:
	void Clear();

	// TODO: Is it needed anymore ?
	void SortNodes(UActionGraphNode* RootNode);
};
