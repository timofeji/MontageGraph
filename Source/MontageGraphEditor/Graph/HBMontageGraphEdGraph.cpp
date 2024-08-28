#include "HBMontageGraphEdGraph.h"


#include "..\MontageGraphEditorLog.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphEdge.h"
#include "MontageGraph/MontageGraphNode_Entry.h"
#include "MontageGraph/MontageGraphNode_Conduit.h"
#include "Nodes/MontageGraphEdNode.h"

#include "Nodes/HBMontageGraphEdNodeConduit.h"
#include "Nodes/HBMontageGraphEdNodeEdge.h"
#include "Nodes/HBMontageGraphEdNodeEntry.h"

UMontageGraph* UHBMontageGraphEdGraph::GetHBMontageGraphModel() const
{
	return CastChecked<UMontageGraph>(GetOuter());
}

void UHBMontageGraphEdGraph::RebuildGraphForConduit(UMontageGraph*              OwningGraph,
                                                   UMontageGraphEdNodeConduit* ConduitNode)
{
	UMontageGraphEdNode*      Node        = Cast<UMontageGraphEdNode>(ConduitNode);
	UMontageGraphNode_Conduit* RuntimeNode = Cast<UMontageGraphNode_Conduit>(ConduitNode->RuntimeNode);

	if (!RuntimeNode)
	{
		return;
	}

	NodeMap.Add(RuntimeNode, ConduitNode);

	OwningGraph->AllNodes.Add(RuntimeNode);

	for (UEdGraphPin* Pin : ConduitNode->Pins)
	{
		if (Pin->Direction != EGPD_Output)
		{
			continue;
		}

		for (const UEdGraphPin* LinkedTo : Pin->LinkedTo)
		{
			UMontageGraphNode* ChildNode = nullptr;

			// Try to determine child node
			if (const UMontageGraphEdNode* OwningNode = Cast<UMontageGraphEdNode>(LinkedTo->GetOwningNode()))
			{
				ChildNode = OwningNode->RuntimeNode;
			}
			else if (const UMontageGraphEdNodeEdge* OwningEdge = Cast<UMontageGraphEdNodeEdge>(
				LinkedTo->GetOwningNode()))
			{
				UMontageGraphEdNode* EndNode = OwningEdge->GetEndNode();
				if (EndNode)
				{
					ChildNode = EndNode->RuntimeNode;
				}
			}

			// Update child / parent nodes for both node and containing ability graph
			if (ChildNode)
			{
				RuntimeNode->ChildrenNodes.Add(ChildNode);
				ChildNode->ParentNodes.Add(RuntimeNode);
			}
		}
	}
}

void UHBMontageGraphEdGraph::RebuildGraph()
{
	MontageGraph_LOG(Verbose, TEXT("UHBMontageGraphEdGraph::RebuildGraph has been called. Nodes Num: %d"), Nodes.Num())

	UMontageGraph* HBMontageGraph = GetHBMontageGraphModel();
	check(HBMontageGraph)

	Clear();

	for (UEdGraphNode* CurrentNode : Nodes)
	{
		MontageGraph_LOG(Verbose, TEXT("UHBMontageGraphEdGraph::RebuildGraph for node: %s (%s)"), *CurrentNode->GetName(),
		              *CurrentNode->GetClass()->GetName())

		if (UMontageGraphEdNodeEntry* GraphEntryNode = Cast<UMontageGraphEdNodeEntry>(CurrentNode))
		{
			RebuildGraphForEntry(HBMontageGraph, GraphEntryNode);
		}
		else if (UMontageGraphEdNodeEdge* GraphEdge = Cast<UMontageGraphEdNodeEdge>(CurrentNode))
		{
			RebuildGraphForEdge(HBMontageGraph, GraphEdge);
		}
		else if (UMontageGraphEdNodeConduit* ConduitNode = Cast<UMontageGraphEdNodeConduit>(CurrentNode))
		{
			RebuildGraphForConduit(HBMontageGraph, ConduitNode);
		}
		else if (UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(CurrentNode))
		{
			RebuildGraphForNode(HBMontageGraph, GraphNode);
		}
	}

	for (UMontageGraphNode* Node : HBMontageGraph->AllNodes)
	{
		if (Node->ParentNodes.Num() == 0)
		{
			// HBMontageGraph->RootNodes.Add(Node);
			// May cause a weird issue, no crash but editor goes unresponsive
			// SortNodes(Node);
		}

		Node->Rename(nullptr, HBMontageGraph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}
}

void UHBMontageGraphEdGraph::RebuildGraphForEdge(UMontageGraph* OwningGraph, UMontageGraphEdNodeEdge* EdGraphEdge)
{
	UMontageGraphEdNode* StartNode = EdGraphEdge->GetStartNodeAsBase();
	UMontageGraphEdNode* EndNode   = EdGraphEdge->GetEndNode();
	UMontageGraphEdge*   Edge      = EdGraphEdge->RuntimeEdge;

	if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
	{
		MontageGraph_LOG(
			Error,
			TEXT("UHBMontageGraphEdGraph::RebuildGraph add edge failed. StartNode: %s, EndNode: %s, Edge: %s"),
			StartNode ? *StartNode->GetName() : TEXT("NONE"),
			EndNode ? *EndNode->GetName() : TEXT("NONE"),
			Edge ? *Edge->GetName() : TEXT("NONE")
		);
		return;
	}

	EdGraphEdge->UpdateCachedIcon();

	EdgeMap.Add(Edge, EdGraphEdge);

	Edge->Rename(nullptr, OwningGraph, REN_DontCreateRedirectors | REN_DoNotDirty);

	if (const UMontageGraphEdNode* Node{Cast<UMontageGraphEdNode>(StartNode)})
	{
		Edge->StartNode = Node->RuntimeNode;
	}
	else if (const UMontageGraphEdNodeConduit* NodeConduit = Cast<UMontageGraphEdNodeConduit>(StartNode))
	{
		Edge->StartNode = NodeConduit->RuntimeNode;
	}

	Edge->EndNode = EndNode->RuntimeNode;

	if (Edge->StartNode)
	{
		Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
	}
}

void UHBMontageGraphEdGraph::RebuildGraphForNode(UMontageGraph* OwningGraph, UMontageGraphEdNode* NodeBase)
{
	UMontageGraphEdNode*        Node    = Cast<UMontageGraphEdNode>(NodeBase);
	UMontageGraphEdNodeConduit* Conduit = Cast<UMontageGraphEdNodeConduit>(NodeBase);

	UMontageGraphNode* RuntimeNode = Node
		                                  ? Node->RuntimeNode
		                                  : Conduit
		                                  ? Conduit->RuntimeNode
		                                  : nullptr;

	if (!RuntimeNode)
	{
		return;
	}

	NodeMap.Add(RuntimeNode, NodeBase);

	OwningGraph->AllNodes.Add(RuntimeNode);

	for (UEdGraphPin* Pin : NodeBase->Pins)
	{
		if (Pin->Direction != EGPD_Output)
		{
			continue;
		}

		for (const UEdGraphPin* LinkedTo : Pin->LinkedTo)
		{
			UMontageGraphNode* ChildNode = nullptr;

			// Try to determine child node
			if (const UMontageGraphEdNode* OwningNode = Cast<UMontageGraphEdNode>(LinkedTo->GetOwningNode()))
			{
				ChildNode = OwningNode->RuntimeNode;
			}
			else if (const UMontageGraphEdNodeEdge* OwningEdge = Cast<UMontageGraphEdNodeEdge>(
				LinkedTo->GetOwningNode()))
			{
				UMontageGraphEdNode* EndNode = OwningEdge->GetEndNode();
				if (EndNode)
				{
					ChildNode = EndNode->RuntimeNode;
				}
			}

			// Update child / parent nodes for both node and containing ability graph
			if (ChildNode)
			{
				RuntimeNode->ChildrenNodes.Add(ChildNode);
				ChildNode->ParentNodes.Add(RuntimeNode);
			}
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UHBMontageGraphEdGraph::RebuildGraphForEntry(UMontageGraph* OwningGraph, UMontageGraphEdNodeEntry* NodeEntry)
{
	check(NodeEntry);
	check(OwningGraph);

	UMontageGraphEdNode*        ConnectedToNode = Cast<UMontageGraphEdNode>(NodeEntry->GetOutputNode());
	UMontageGraphEdNodeConduit* Conduit         = Cast<UMontageGraphEdNodeConduit>(NodeEntry->GetOutputNode());

	MontageGraph_LOG(Verbose, TEXT("UHBMontageGraphEdGraph::RebuildGraphForEntry ... Node: %s"),
				  ConnectedToNode ? *ConnectedToNode->GetName() : TEXT("NONE"))
	MontageGraph_LOG(Verbose, TEXT("UHBMontageGraphEdGraph::RebuildGraphForEntry ... Conduit: %s"),
				  Conduit ? *Conduit->GetName() : TEXT("NONE"))

	if (ConnectedToNode)
	{
		NodeEntry->RuntimeNode->ChildrenNodes.Add(ConnectedToNode->RuntimeNode);
		ConnectedToNode->RuntimeNode->ParentNodes.Add(NodeEntry->RuntimeNode);
	}
	
	if(auto	EntryNode {Cast<UMontageGraphNode_Entry>(NodeEntry->RuntimeNode)})
	{
		// Setup Entry node reference (created by schema when first creating the graph)
		OwningGraph->EntryNodes.Add(EntryNode->EntryTag	, NodeEntry->RuntimeNode);
	}
}

void UHBMontageGraphEdGraph::ValidateNodes(FCompilerResultsLog* LogResults)
{
	for (const UEdGraphNode* Node : Nodes)
	{
		if (const UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(Node))
		{
			Node->ValidateNodeDuringCompilation(*LogResults);
		}
	}
}

bool UHBMontageGraphEdGraph::Modify(const bool bAlwaysMarkDirty)
{
	const bool bWasSaved = Super::Modify(bAlwaysMarkDirty);

	UMontageGraph* HBMontageGraphModel = GetHBMontageGraphModel();
	if (HBMontageGraphModel)
	{
		HBMontageGraphModel->Modify();
	}

	for (UEdGraphNode* Node : Nodes)
	{
		Node->Modify();
	}

	return bWasSaved;
}

void UHBMontageGraphEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
}

TArray<UMontageGraphEdNode*> UHBMontageGraphEdGraph::GetAllNodes() const
{
	TArray<UMontageGraphEdNode*> OutNodes;
	for (UEdGraphNode* EdNode : Nodes)
	{
		if (UMontageGraphEdNode* Node = Cast<UMontageGraphEdNode>(EdNode))
		{
			OutNodes.Add(Node);
		}
	}

	return OutNodes;
}

namespace ACEAutoArrangeHelpers
{
	struct FNodeBoundsInfo
	{
		FVector2D               SubGraphBBox;
		TArray<FNodeBoundsInfo> Children;
	};

	UEdGraphPin* FindGraphNodePin(UEdGraphNode* Node, const EEdGraphPinDirection Dir)
	{
		UEdGraphPin* Pin = nullptr;
		for (int32 Idx = 0; Idx < Node->Pins.Num(); Idx++)
		{
			if (Node->Pins[Idx]->Direction == Dir)
			{
				Pin = Node->Pins[Idx];
				break;
			}
		}

		return Pin;
	}

	void AutoArrangeNodesVertically(UMontageGraphEdNode* ParentNode, FNodeBoundsInfo& BBoxTree, float PosX, float PosY)
	{
		int32 BBoxIndex = 0;

		UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			SGraphNode::FNodeSet NodeFilter;
			TArray<UEdGraphPin*> TempLinkedTo = Pin->LinkedTo;
			for (int32 Idx = 0; Idx < TempLinkedTo.Num(); Idx++)
			{
				UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(TempLinkedTo[Idx]->GetOwningNode());
				if (GraphNode && BBoxTree.Children.Num() > 0)
				{
					AutoArrangeNodesVertically(GraphNode, BBoxTree.Children[BBoxIndex], PosX,
					                           PosY + GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y *
					                           2.5f);
					GraphNode->DEPRECATED_NodeWidget.Pin()->MoveTo(
						FVector2D(
							BBoxTree.Children[BBoxIndex].SubGraphBBox.X / 2 - GraphNode->DEPRECATED_NodeWidget.Pin()->
							GetDesiredSize().X / 2 + PosX, PosY), NodeFilter);
					PosX += BBoxTree.Children[BBoxIndex].SubGraphBBox.X + 20;
					BBoxIndex++;
				}
			}
		}
	}

	void AutoArrangeNodesHorizontally(UMontageGraphEdNode* ParentNode, FNodeBoundsInfo& BBoxTree, float PosX,
	                                  float                 PosY)
	{
		int32 BBoxIndex = 0;

		UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			SGraphNode::FNodeSet NodeFilter;
			TArray<UEdGraphPin*> TempLinkedTo = Pin->LinkedTo;
			for (int32 Idx = 0; Idx < TempLinkedTo.Num(); Idx++)
			{
				UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(TempLinkedTo[Idx]->GetOwningNode());
				if (GraphNode && BBoxTree.Children.Num() > 0)
				{
					// AutoArrangeNodesVertically(GraphNode, BBoxTree.Children[BBoxIndex], PosX, PosY + GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y * 2.5f);
					AutoArrangeNodesHorizontally(GraphNode, BBoxTree.Children[BBoxIndex],
					                             PosX + GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X *
					                             2.5f, PosY);

					// GraphNode->DEPRECATED_NodeWidget.Pin()->MoveTo(FVector2D(BBoxTree.Children[BBoxIndex].SubGraphBBox.X / 2 - GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X / 2 + PosX, PosY), NodeFilter);
					GraphNode->DEPRECATED_NodeWidget.Pin()->MoveTo(
						FVector2D(
							PosX, BBoxTree.Children[BBoxIndex].SubGraphBBox.Y / 2 - GraphNode->DEPRECATED_NodeWidget.
							Pin()->GetDesiredSize().Y / 2 + PosY), NodeFilter);

					// PosX += BBoxTree.Children[BBoxIndex].SubGraphBBox.X + 20;
					PosY += BBoxTree.Children[BBoxIndex].SubGraphBBox.Y + 20;
					BBoxIndex++;
				}
			}
		}
	}

	void GetNodeSizeInfo(UMontageGraphEdNode*          ParentNode, FNodeBoundsInfo& BBoxTree,
	                     TArray<UMontageGraphEdNode*>& VisitedNodes)
	{
		BBoxTree.SubGraphBBox = ParentNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize();
		float LevelWidth      = 0;
		float LevelHeight     = 0;

		UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			// Pin->LinkedTo.Sort(FHBMontageGraphCompareNodeXLocation());
			for (int32 Idx = 0; Idx < Pin->LinkedTo.Num(); Idx++)
			{
				UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(Pin->LinkedTo[Idx]->GetOwningNode());
				if (GraphNode)
				{
					const int32      ChildIdx    = BBoxTree.Children.Add(FNodeBoundsInfo());
					FNodeBoundsInfo& ChildBounds = BBoxTree.Children[ChildIdx];

					if (!VisitedNodes.Contains(GraphNode))
					{
						VisitedNodes.AddUnique(GraphNode);
						GetNodeSizeInfo(GraphNode, ChildBounds, VisitedNodes);
					}

					LevelWidth += ChildBounds.SubGraphBBox.X + 20;
					if (ChildBounds.SubGraphBBox.Y > LevelHeight)
					{
						LevelHeight = ChildBounds.SubGraphBBox.Y;
					}
				}
			}

			if (LevelWidth > BBoxTree.SubGraphBBox.X)
			{
				BBoxTree.SubGraphBBox.X = LevelWidth;
			}

			BBoxTree.SubGraphBBox.Y += LevelHeight;
		}
	}
}


void UHBMontageGraphEdGraph::AutoArrange(const bool bVertical)
{
	UMontageGraphEdNode* RootNode = nullptr;
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		RootNode = Cast<UMontageGraphEdNodeEntry>(Nodes[Idx]);
		if (RootNode)
		{
			break;
		}
	}

	if (!RootNode)
	{
		return;
	}

	MontageGraph_LOG(Verbose, TEXT("UHBMontageGraphEdGraph::AutoArrange Strategy: %s"),
	              bVertical ? TEXT("Vertical") : TEXT("Horizontal"))
	const FScopedTransaction Transaction(NSLOCTEXT("ACEGraph", "CombatGraphEditorAutoArrange",
	                                               "Montage Graph Editor: Auto Arrange"));

	ACEAutoArrangeHelpers::FNodeBoundsInfo BBoxTree;
	TArray<UMontageGraphEdNode*>          VisitedNodes;
	ACEAutoArrangeHelpers::GetNodeSizeInfo(RootNode, BBoxTree, VisitedNodes);

	SGraphNode::FNodeSet NodeFilter;
	if (bVertical)
	{
		ACEAutoArrangeHelpers::AutoArrangeNodesVertically(RootNode, BBoxTree, 0,
		                                                  RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y *
		                                                  2.5f);

		const float NewRootPosX = BBoxTree.SubGraphBBox.X / 2 - RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().
		                                                                  X / 2;
		RootNode->DEPRECATED_NodeWidget.Pin()->MoveTo(FVector2D(NewRootPosX, 0), NodeFilter);
	}
	else
	{
		ACEAutoArrangeHelpers::AutoArrangeNodesHorizontally(RootNode, BBoxTree,
		                                                    RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X *
		                                                    2.5f, 0);

		const float NewRootPosY = BBoxTree.SubGraphBBox.Y / 2 - RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().
		                                                                  Y / 2;
		RootNode->DEPRECATED_NodeWidget.Pin()->MoveTo(FVector2D(0, NewRootPosY), NodeFilter);
	}

	RootNode->DEPRECATED_NodeWidget.Pin()->GetOwnerPanel()->ZoomToFit(/*bOnlySelection=*/ false);
}

void UHBMontageGraphEdGraph::Clear()
{
	UMontageGraph* HBMontageGraphModel = GetHBMontageGraphModel();
	if (HBMontageGraphModel)
	{
		HBMontageGraphModel->ClearGraph();
	}

	NodeMap.Reset();
	EdgeMap.Reset();
	EntryNodes.Reset();

	for (UEdGraphNode* Node : Nodes)
	{
		if (const UMontageGraphEdNode* EdGraphNode = Cast<UMontageGraphEdNode>(Node))
		{
			UMontageGraphNode* GraphNode = EdGraphNode->RuntimeNode;
			if (GraphNode)
			{
				GraphNode->ParentNodes.Reset();
				GraphNode->ChildrenNodes.Reset();
				GraphNode->Edges.Reset();
			}
		}
	}
}

void UHBMontageGraphEdGraph::SortNodes(UMontageGraphNode* RootNode)
{
	TArray<UMontageGraphNode*> CurrLevelNodes = {RootNode};
	TArray<UMontageGraphNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (UMontageGraphNode* Node : CurrLevelNodes)
		{
			auto Comp = [&](const UMontageGraphNode& L, const UMontageGraphNode& R)
			{
				const UMontageGraphEdNode* EdNode_LNode = NodeMap[&L];
				const UMontageGraphEdNode* EdNode_RNode = NodeMap[&R];
				return EdNode_LNode->NodePosX < EdNode_RNode->NodePosX;
			};

			Node->ChildrenNodes.Sort(Comp);
			Node->ParentNodes.Sort(Comp);

			NextLevelNodes.Append(Node->ChildrenNodes);
		}

		CurrLevelNodes = NextLevelNodes;
		NextLevelNodes.Reset();
	}
}