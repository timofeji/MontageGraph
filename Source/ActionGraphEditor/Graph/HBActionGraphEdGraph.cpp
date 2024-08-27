#include "HBActionGraphEdGraph.h"


#include "..\ActionGraphEditorLog.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"
#include "ActionGraph/ActionGraph.h"
#include "ActionGraph/ActionGraphEdge.h"
#include "ActionGraph/ActionGraphNode_Entry.h"
#include "ActionGraph/ActionGraphNode_Conduit.h"
#include "Nodes/ActionGraphEdNode.h"

#include "Nodes/HBActionGraphEdNodeConduit.h"
#include "Nodes/HBActionGraphEdNodeEdge.h"
#include "Nodes/HBActionGraphEdNodeEntry.h"

UActionGraph* UHBActionGraphEdGraph::GetHBActionGraphModel() const
{
	return CastChecked<UActionGraph>(GetOuter());
}

void UHBActionGraphEdGraph::RebuildGraphForConduit(UActionGraph*              OwningGraph,
                                                   UActionGraphEdNodeConduit* ConduitNode)
{
	UActionGraphEdNode*      Node        = Cast<UActionGraphEdNode>(ConduitNode);
	UActionGraphNode_Conduit* RuntimeNode = Cast<UActionGraphNode_Conduit>(ConduitNode->RuntimeNode);

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
			UActionGraphNode* ChildNode = nullptr;

			// Try to determine child node
			if (const UActionGraphEdNode* OwningNode = Cast<UActionGraphEdNode>(LinkedTo->GetOwningNode()))
			{
				ChildNode = OwningNode->RuntimeNode;
			}
			else if (const UActionGraphEdNodeEdge* OwningEdge = Cast<UActionGraphEdNodeEdge>(
				LinkedTo->GetOwningNode()))
			{
				UActionGraphEdNode* EndNode = OwningEdge->GetEndNode();
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

void UHBActionGraphEdGraph::RebuildGraph()
{
	ACTIONGRAPH_LOG(Verbose, TEXT("UHBActionGraphEdGraph::RebuildGraph has been called. Nodes Num: %d"), Nodes.Num())

	UActionGraph* HBActionGraph = GetHBActionGraphModel();
	check(HBActionGraph)

	Clear();

	for (UEdGraphNode* CurrentNode : Nodes)
	{
		ACTIONGRAPH_LOG(Verbose, TEXT("UHBActionGraphEdGraph::RebuildGraph for node: %s (%s)"), *CurrentNode->GetName(),
		              *CurrentNode->GetClass()->GetName())

		if (UActionGraphEdNodeEntry* GraphEntryNode = Cast<UActionGraphEdNodeEntry>(CurrentNode))
		{
			RebuildGraphForEntry(HBActionGraph, GraphEntryNode);
		}
		else if (UActionGraphEdNodeEdge* GraphEdge = Cast<UActionGraphEdNodeEdge>(CurrentNode))
		{
			RebuildGraphForEdge(HBActionGraph, GraphEdge);
		}
		else if (UActionGraphEdNodeConduit* ConduitNode = Cast<UActionGraphEdNodeConduit>(CurrentNode))
		{
			RebuildGraphForConduit(HBActionGraph, ConduitNode);
		}
		else if (UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(CurrentNode))
		{
			RebuildGraphForNode(HBActionGraph, GraphNode);
		}
	}

	for (UActionGraphNode* Node : HBActionGraph->AllNodes)
	{
		if (Node->ParentNodes.Num() == 0)
		{
			// HBActionGraph->RootNodes.Add(Node);
			// May cause a weird issue, no crash but editor goes unresponsive
			// SortNodes(Node);
		}

		Node->Rename(nullptr, HBActionGraph, REN_DontCreateRedirectors | REN_DoNotDirty);
	}
}

void UHBActionGraphEdGraph::RebuildGraphForEdge(UActionGraph* OwningGraph, UActionGraphEdNodeEdge* EdGraphEdge)
{
	UActionGraphEdNode* StartNode = EdGraphEdge->GetStartNodeAsBase();
	UActionGraphEdNode* EndNode   = EdGraphEdge->GetEndNode();
	UActionGraphEdge*   Edge      = EdGraphEdge->RuntimeEdge;

	if (StartNode == nullptr || EndNode == nullptr || Edge == nullptr)
	{
		ACTIONGRAPH_LOG(
			Error,
			TEXT("UHBActionGraphEdGraph::RebuildGraph add edge failed. StartNode: %s, EndNode: %s, Edge: %s"),
			StartNode ? *StartNode->GetName() : TEXT("NONE"),
			EndNode ? *EndNode->GetName() : TEXT("NONE"),
			Edge ? *Edge->GetName() : TEXT("NONE")
		);
		return;
	}

	EdGraphEdge->UpdateCachedIcon();

	EdgeMap.Add(Edge, EdGraphEdge);

	Edge->Rename(nullptr, OwningGraph, REN_DontCreateRedirectors | REN_DoNotDirty);

	if (const UActionGraphEdNode* Node{Cast<UActionGraphEdNode>(StartNode)})
	{
		Edge->StartNode = Node->RuntimeNode;
	}
	else if (const UActionGraphEdNodeConduit* NodeConduit = Cast<UActionGraphEdNodeConduit>(StartNode))
	{
		Edge->StartNode = NodeConduit->RuntimeNode;
	}

	Edge->EndNode = EndNode->RuntimeNode;

	if (Edge->StartNode)
	{
		Edge->StartNode->Edges.Add(Edge->EndNode, Edge);
	}
}

void UHBActionGraphEdGraph::RebuildGraphForNode(UActionGraph* OwningGraph, UActionGraphEdNode* NodeBase)
{
	UActionGraphEdNode*        Node    = Cast<UActionGraphEdNode>(NodeBase);
	UActionGraphEdNodeConduit* Conduit = Cast<UActionGraphEdNodeConduit>(NodeBase);

	UActionGraphNode* RuntimeNode = Node
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
			UActionGraphNode* ChildNode = nullptr;

			// Try to determine child node
			if (const UActionGraphEdNode* OwningNode = Cast<UActionGraphEdNode>(LinkedTo->GetOwningNode()))
			{
				ChildNode = OwningNode->RuntimeNode;
			}
			else if (const UActionGraphEdNodeEdge* OwningEdge = Cast<UActionGraphEdNodeEdge>(
				LinkedTo->GetOwningNode()))
			{
				UActionGraphEdNode* EndNode = OwningEdge->GetEndNode();
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
void UHBActionGraphEdGraph::RebuildGraphForEntry(UActionGraph* OwningGraph, UActionGraphEdNodeEntry* NodeEntry)
{
	check(NodeEntry);
	check(OwningGraph);

	UActionGraphEdNode*        ConnectedToNode = Cast<UActionGraphEdNode>(NodeEntry->GetOutputNode());
	UActionGraphEdNodeConduit* Conduit         = Cast<UActionGraphEdNodeConduit>(NodeEntry->GetOutputNode());

	ACTIONGRAPH_LOG(Verbose, TEXT("UHBActionGraphEdGraph::RebuildGraphForEntry ... Node: %s"),
				  ConnectedToNode ? *ConnectedToNode->GetName() : TEXT("NONE"))
	ACTIONGRAPH_LOG(Verbose, TEXT("UHBActionGraphEdGraph::RebuildGraphForEntry ... Conduit: %s"),
				  Conduit ? *Conduit->GetName() : TEXT("NONE"))

	if (ConnectedToNode)
	{
		NodeEntry->RuntimeNode->ChildrenNodes.Add(ConnectedToNode->RuntimeNode);
		ConnectedToNode->RuntimeNode->ParentNodes.Add(NodeEntry->RuntimeNode);
	}
	
	if(auto	EntryNode {Cast<UActionGraphNode_Entry>(NodeEntry->RuntimeNode)})
	{
		// Setup Entry node reference (created by schema when first creating the graph)
		OwningGraph->EntryNodes.Add(EntryNode->EntryTag	, NodeEntry->RuntimeNode);
	}
}

void UHBActionGraphEdGraph::ValidateNodes(FCompilerResultsLog* LogResults)
{
	for (const UEdGraphNode* Node : Nodes)
	{
		if (const UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(Node))
		{
			Node->ValidateNodeDuringCompilation(*LogResults);
		}
	}
}

bool UHBActionGraphEdGraph::Modify(const bool bAlwaysMarkDirty)
{
	const bool bWasSaved = Super::Modify(bAlwaysMarkDirty);

	UActionGraph* HBActionGraphModel = GetHBActionGraphModel();
	if (HBActionGraphModel)
	{
		HBActionGraphModel->Modify();
	}

	for (UEdGraphNode* Node : Nodes)
	{
		Node->Modify();
	}

	return bWasSaved;
}

void UHBActionGraphEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
}

TArray<UActionGraphEdNode*> UHBActionGraphEdGraph::GetAllNodes() const
{
	TArray<UActionGraphEdNode*> OutNodes;
	for (UEdGraphNode* EdNode : Nodes)
	{
		if (UActionGraphEdNode* Node = Cast<UActionGraphEdNode>(EdNode))
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

	void AutoArrangeNodesVertically(UActionGraphEdNode* ParentNode, FNodeBoundsInfo& BBoxTree, float PosX, float PosY)
	{
		int32 BBoxIndex = 0;

		UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			SGraphNode::FNodeSet NodeFilter;
			TArray<UEdGraphPin*> TempLinkedTo = Pin->LinkedTo;
			for (int32 Idx = 0; Idx < TempLinkedTo.Num(); Idx++)
			{
				UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(TempLinkedTo[Idx]->GetOwningNode());
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

	void AutoArrangeNodesHorizontally(UActionGraphEdNode* ParentNode, FNodeBoundsInfo& BBoxTree, float PosX,
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
				UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(TempLinkedTo[Idx]->GetOwningNode());
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

	void GetNodeSizeInfo(UActionGraphEdNode*          ParentNode, FNodeBoundsInfo& BBoxTree,
	                     TArray<UActionGraphEdNode*>& VisitedNodes)
	{
		BBoxTree.SubGraphBBox = ParentNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize();
		float LevelWidth      = 0;
		float LevelHeight     = 0;

		UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			// Pin->LinkedTo.Sort(FHBActionGraphCompareNodeXLocation());
			for (int32 Idx = 0; Idx < Pin->LinkedTo.Num(); Idx++)
			{
				UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(Pin->LinkedTo[Idx]->GetOwningNode());
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


void UHBActionGraphEdGraph::AutoArrange(const bool bVertical)
{
	UActionGraphEdNode* RootNode = nullptr;
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		RootNode = Cast<UActionGraphEdNodeEntry>(Nodes[Idx]);
		if (RootNode)
		{
			break;
		}
	}

	if (!RootNode)
	{
		return;
	}

	ACTIONGRAPH_LOG(Verbose, TEXT("UHBActionGraphEdGraph::AutoArrange Strategy: %s"),
	              bVertical ? TEXT("Vertical") : TEXT("Horizontal"))
	const FScopedTransaction Transaction(NSLOCTEXT("ACEGraph", "CombatGraphEditorAutoArrange",
	                                               "Action Graph Editor: Auto Arrange"));

	ACEAutoArrangeHelpers::FNodeBoundsInfo BBoxTree;
	TArray<UActionGraphEdNode*>          VisitedNodes;
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

void UHBActionGraphEdGraph::Clear()
{
	UActionGraph* HBActionGraphModel = GetHBActionGraphModel();
	if (HBActionGraphModel)
	{
		HBActionGraphModel->ClearGraph();
	}

	NodeMap.Reset();
	EdgeMap.Reset();
	EntryNodes.Reset();

	for (UEdGraphNode* Node : Nodes)
	{
		if (const UActionGraphEdNode* EdGraphNode = Cast<UActionGraphEdNode>(Node))
		{
			UActionGraphNode* GraphNode = EdGraphNode->RuntimeNode;
			if (GraphNode)
			{
				GraphNode->ParentNodes.Reset();
				GraphNode->ChildrenNodes.Reset();
				GraphNode->Edges.Reset();
			}
		}
	}
}

void UHBActionGraphEdGraph::SortNodes(UActionGraphNode* RootNode)
{
	TArray<UActionGraphNode*> CurrLevelNodes = {RootNode};
	TArray<UActionGraphNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (UActionGraphNode* Node : CurrLevelNodes)
		{
			auto Comp = [&](const UActionGraphNode& L, const UActionGraphNode& R)
			{
				const UActionGraphEdNode* EdNode_LNode = NodeMap[&L];
				const UActionGraphEdNode* EdNode_RNode = NodeMap[&R];
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
