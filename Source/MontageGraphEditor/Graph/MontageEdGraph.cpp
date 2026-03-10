#include "MontageEdGraph.h"


#include "..\MontageGraphEditorLog.h"
#include "SGraphNode.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/Nodes/MGEdge.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"
#include "MontageGraph/Nodes/MontageGraphNode_Entry.h"
#include "MontageGraph/Nodes/MGNode_Selector.h"
#include "EdNodes/MGEdNode.h"

#include "EdNodes/MGEdNode_Selector.h"
#include "EdNodes/MGEdNode_Edge.h"
#include "EdNodes/MGEdNode_Entry.h"
#include "EdNodes/MGEdNode_Montage.h"

#define LOCTEXT_NAMESPACE "UMontageGraphEdGraph"

UMontageGraph* UMontageEdGraph::GetMontageGraphModel() const
{
	return CastChecked<UMontageGraph>(GetOuter());
}



void UMontageEdGraph::RebuildRuntimeGraph()
{
	MG_LOG("GraphRebuild has been called. Nodes Num: %d", Nodes.Num())

	UMontageGraph* OwningGraph = GetMontageGraphModel();
	check(OwningGraph)

	Clear();


	for (UEdGraphNode* CurrentNode : Nodes)
	{
		if (UMGEdNode_Entry* EntryEdNode = Cast<UMGEdNode_Entry>(CurrentNode))
		{
			if (auto EntryNode{Cast<UMontageGraphNode_Entry>(EntryEdNode->RuntimeNode)})
			{
				RebuildMontageTreeForEntryBind(EntryEdNode, OwningGraph);
				OwningGraph->RootNodes.Add(EntryNode->EntryTag, EntryNode);
			}
		}
	}

	MapGraphAggregateTags(OwningGraph);


	// Iterate over all objects in the outer & Force garbage collection on unused objects
	ForEachObjectWithOuter(OwningGraph, [&](UObject* Obj)
	{
		if (Obj->IsA(UAnimMontage::StaticClass()))
		{
			if (!OwningGraph->Montages.Contains(Obj))
			{
				Obj->ClearFlags(RF_Public | RF_Standalone);
				Obj->MarkAsGarbage();
				// Obj->ConditionalBeginDestroy(); 
			}
		}
		return true; // continue iteration
	}, /*bIncludeNestedObjects=*/ true);

	CollectGarbage(RF_NoFlags);

	
}


void UMontageEdGraph::RebuildGraphForSelector(UMontageGraph* OwningGraph,
                                                   UMGEdNode_Selector* EdNode)
{
	UMGNode_Selector* SelectorNode = Cast<UMGNode_Selector>(EdNode->RuntimeNode);

	if (!SelectorNode || SelectorNode->SelectorStates.IsEmpty())
	{
		return;
	}

	NodeMap.Add(SelectorNode, EdNode);

	TArray<FGameplayTag> FinalSelectorStates;
	for (int i = 0; i < SelectorNode->SelectorStates.Num(); i++)
	{
		//Skip input Pin(i+1)
		if (!EdNode->Pins.IsValidIndex(i + 1))
		{
			UE_LOG(LogMontageGraphEditor, Error, TEXT("[%s]: SELECTOR NODE HAS INCORRECT AMOUNT OF PINS"),
			       *GetFullNameSafe(this));
			return;
		}

		UEdGraphPin* Pin = EdNode->Pins[i + 1];
		if (Pin->LinkedTo.Num() > 0)
		{
			if (UEdGraphPin* LinkedTo = Pin->LinkedTo[0])
			{
				UMGNode* ChildNode = nullptr;
				// Try to determine child node
				if (const UMGEdNode* OwningNode = Cast<UMGEdNode>(LinkedTo->GetOwningNode()))
				{
					ChildNode = OwningNode->RuntimeNode;
				}
				else if (const UMGEdNode_Edge* OwningEdge = Cast<UMGEdNode_Edge>(
					LinkedTo->GetOwningNode()))
				{
					UMGEdNode* EndNode = OwningEdge->GetEndNode();
					if (EndNode)
					{
						ChildNode = EndNode->RuntimeNode;
					}
				}

				// Update child / parent nodes for both node and containing ability graph
				if (ChildNode)
				{
					SelectorNode->ChildrenNodes.Add(ChildNode);
					ChildNode->ParentNodes.Add(SelectorNode);
					FinalSelectorStates.Add(SelectorNode->SelectorStates[i]);
				}
			}
		}
	}
	
	
	SelectorNode->SelectorStates = FinalSelectorStates;

	// for (UEdGraphPin* Pin : SelectorNode->Pins)
	// {
	// 	if (Pin->Direction != EGPD_Output)
	// 	{
	// 		continue;
	// 	}
	//
	// 	for (const UEdGraphPin* LinkedTo : Pin->LinkedTo)
	// 	{
	// 	}
	// }
}

void UMontageEdGraph::MapGraphAggregateTags(UMontageGraph* OwningGraph)
{
	if (!OwningGraph)
	{
		return;
	}

	for (auto RootNode : OwningGraph->RootNodes)
	{
		FGameplayTagContainer BindTags;
		BindTags.AddTag(RootNode.Key);
		MapSubGraph(RootNode.Value, BindTags);
	}
}

void UMontageEdGraph::MapSubGraph(UMGNode* CurNode, FGameplayTagContainer TraversedTags)
{
	if (UMGNode_Montage* AnimNode = Cast<UMGNode_Montage>(CurNode))
	{
		AnimNode->AggregatedTags = TraversedTags;
	}

	
	for (auto NodeEdge : CurNode->Edges)
	{
		FGameplayTagContainer EdgeTags = FGameplayTagContainer(TraversedTags);
		
		if (auto TagQueryEdge = Cast<UMGEdge_TagQuery>(NodeEdge))
		{
			FGameplayTagQueryExpression Expr;
			TagQueryEdge->Query.GetQueryExpr(Expr);
			
			EdgeTags.AppendTags(FGameplayTagContainer::CreateFromArray(TagQueryEdge->Query.GetGameplayTagArray()));
			for (auto ExprSet : Expr.ExprSet)
			{
				if (ExprSet.ExprType == EGameplayTagQueryExprType::NoTagsMatch || ExprSet.ExprType == EGameplayTagQueryExprType::NoExprMatch)
				{
					EdgeTags.RemoveTags(FGameplayTagContainer::CreateFromArray(ExprSet.TagSet));
				}
			}
		}

		if (auto StackCountEdge = Cast<UMGEdge_GameplayEffectQuery>(NodeEdge))
		{
			// (StackCountEdge->Query.);
			//
			// if (StackCount >= StackCountEdge->RequiredNumOfStacks)
			// {
			// 	NodeToTransitionTo = QueryNode->ChildrenNodes[i];
			// 	break;
			// }
		}

		MapSubGraph(NodeEdge->EndNode, EdgeTags);
	}
}

void UMontageEdGraph::RebuildStaleMontages()
{
	for (UEdGraphNode* CurrentNode : Nodes)
	{
		if (UMGEdNode_Montage* MontageEdNode = Cast<UMGEdNode_Montage>(CurrentNode))
		{
			if (MontageEdNode->bShouldRegenerate)
			{
				// MontageEdNode->RegenerateMontage(GetMontageGraphModel(), MontageEdNode->RuntimeNode);
			}
		}
	}
}

void UMontageEdGraph::RebuildGraphForEdge(UMontageGraph* OwningGraph, UMGEdNode_Edge* EdGraphEdge)
{
	UMGEdNode* StartEdNode = EdGraphEdge->GetStartNode();
	UMGEdNode* EndEdNode   = EdGraphEdge->GetEndNode();
	UMGNode*   StartNode   = StartEdNode->GetRuntimeNode();
	UMGNode*   EndNode     = EndEdNode->GetRuntimeNode();
	UMGEdge*   Edge        = EdGraphEdge->RuntimeEdge;

	if (StartEdNode == nullptr || EndEdNode == nullptr || Edge == nullptr)
	{
		return;
	}
	Edge->Rename(nullptr, OwningGraph, REN_DontCreateRedirectors | REN_DoNotDirty);

	Edge->StartNode = StartNode;
	StartNode->Rename(nullptr, OwningGraph, REN_DontCreateRedirectors | REN_DoNotDirty);
	
	Edge->EndNode = EndNode;
	EndNode->Rename(nullptr, OwningGraph, REN_DontCreateRedirectors | REN_DoNotDirty);
	
	StartNode->Edges.Add(Edge);
	StartNode->ChildrenNodes.Add(EndNode);
	EndNode->ParentNodes.Add(StartNode);
	

	EdgeMap.Add(Edge, EdGraphEdge);
	
	EdGraphEdge->UpdateCachedIcon();
}

void UMontageEdGraph::RebuildMontageTreeForEntryBind(UMGEdNode* RootNode, UMontageGraph* OwningGraph)
{
	//First, find the nodes that are actually connected to the entry bind
	TArray<UMGEdNode_Montage*> MontageEdNodes;
	RebuildNodeTree(RootNode, OwningGraph, MontageEdNodes);
	//
	// for (UEdGraphPin* Pin : RootNode->Pins)
	// {
	// 	if (Pin->Direction != EGPD_Output)
	// 	{
	// 		continue;
	// 	}
	//
	// 	for (const UEdGraphPin* LinkedToPin : Pin->LinkedTo)
	// 	{
	// 		UMGEdNode* ConnectedToNode = Cast<UMGEdNode>(LinkedToPin->GetOwningNode());
	// 		if (ConnectedToNode)
	// 		{
	// 		}
	// 	}
	// }




	FScopedSlowTask RebuildGraphSlowTask(MontageEdNodes.Num(),
	                                     LOCTEXT("RebuildGraphSlowTask", "Rebuilding MontageGraph..."), true);
	bool bShowCancelButton = false;
	bool bAllowInPIE = true;
	RebuildGraphSlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);


	uint16 NodeID = OwningGraph->Montages.Num();
	for (int i = 0; i < MontageEdNodes.Num(); i++)
	{
		RebuildGraphSlowTask.EnterProgressFrame(1.f, LOCTEXT("Rebuilding Montage", "Rebuilding Montage"));
		RebuildGraphSlowTask.ForceRefresh();

		UMGEdNode_Montage*    MontageEdNode = MontageEdNodes[i];
		UMGNode_Montage*      MontageNode   = Cast<UMGNode_Montage>(MontageEdNode->RuntimeNode);
		MontageNode->ID = NodeID;
		MontageNode->SetNodeTitle(FText::FromString(MontageEdNode->MontageDisplayName));
		
		OwningGraph->MontageNodes.Add(MontageNode);
		MontageEdNode->RegenerateMontage(OwningGraph, MontageNode);

		NodeID++;
	}
}

void UMontageEdGraph::RebuildNodeTree(
	UMGEdNode*                  RootNode,
	UMontageGraph*              OwningGraph,
	TArray<UMGEdNode_Montage*>& MontageEdNodes)
{
	UMGEdNode* CurEdNode = RootNode;
	UMGNode*   CurNode   = CurEdNode->RuntimeNode;
	



	if (auto MontageEdNode = Cast<UMGEdNode_Montage>(CurEdNode))
	{
		if (MontageEdNodes.Contains(MontageEdNode))
		{
			return;	
		}
		
		MontageEdNodes.Add(MontageEdNode);
	}


	for (UEdGraphPin* Pin : CurEdNode->Pins)
	{
		if (Pin->Direction != EGPD_Output)
		{
			continue;
		}

		for (const UEdGraphPin* LinkedToPin : Pin->LinkedTo)
		{
			if (auto EdgeEdNode = Cast<UMGEdNode_Edge>(LinkedToPin->GetOwningNode()))
			{
				RebuildGraphForEdge(OwningGraph, EdgeEdNode);
				RebuildNodeTree(EdgeEdNode->GetEndNode(), OwningGraph, MontageEdNodes);
			}
			else if (auto ChildEdNode = Cast<UMGEdNode>(LinkedToPin->GetOwningNode()))
			{
				UMGNode* ChildNode = ChildEdNode->RuntimeNode;
				if (ChildNode)
				{
					CurNode->ChildrenNodes.Add(ChildNode);
					ChildNode->ParentNodes.Add(CurNode);
					RebuildNodeTree(ChildEdNode, OwningGraph, MontageEdNodes);
				}
			}
		}
	}
}

void UMontageEdGraph::MapLinkedMontages(UMGEdNode_Montage* RootNode,  TArray<UMGNode_Montage*>& OutMontages)
{
	UMGNode_Montage* MontageNode = Cast<UMGNode_Montage>(RootNode->RuntimeNode);
	if (!MontageNode)
	{
		MG_ERROR("Attempting to map a montage node with no runtime node, THIS SHOULD NEVER HAPPEN");
		return;	
	}

	OutMontages.Add(MontageNode);

	for (UEdGraphPin* Pin : RootNode->Pins)
	{
		if (Pin->Direction != EGPD_Output)
		{
			continue;
		}

		for (const UEdGraphPin* LinkedToPin : Pin->LinkedTo)
		{
			if (UMGEdNode_Montage* MontageEdNode = Cast<UMGEdNode_Montage>(LinkedToPin->GetOwningNode()))
			{
				MapLinkedMontages(MontageEdNode , OutMontages);
			}
		}
	}
}

void UMontageEdGraph::ValidateNodes(FCompilerResultsLog* LogResults)
{
	for (const UEdGraphNode* Node : Nodes)
	{
		if (const UMGEdNode* GraphNode = Cast<UMGEdNode>(Node))
		{
			Node->ValidateNodeDuringCompilation(*LogResults);
		}
	}
}

bool UMontageEdGraph::Modify(const bool bAlwaysMarkDirty)
{
	const bool bWasSaved = Super::Modify(bAlwaysMarkDirty);

	UMontageGraph* MontageGraphModel = GetMontageGraphModel();
	if (MontageGraphModel)
	{
		MontageGraphModel->Modify();
	}

	for (UEdGraphNode* Node : Nodes)
	{
		Node->Modify();
	}

	return bWasSaved;
}

void UMontageEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
}

TArray<UMGEdNode*> UMontageEdGraph::GetAllNodes() const
{
	TArray<UMGEdNode*> OutNodes;
	for (UEdGraphNode* EdNode : Nodes)
	{
		if (UMGEdNode* Node = Cast<UMGEdNode>(EdNode))
		{
			OutNodes.Add(Node);
		}
	}

	return OutNodes;
}

namespace MGAutoArrangeHelpers
{
	struct FNodeBoundsInfo
	{
		FVector2D SubGraphBBox;
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

	void AutoArrangeNodesVertically(UMGEdNode* ParentNode, FNodeBoundsInfo& BBoxTree, float PosX, float PosY)
	{
		int32 BBoxIndex = 0;
		//
		// UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		// if (Pin)
		// {
		// 	SGraphNode::FNodeSet NodeFilter;
		// 	TArray<UEdGraphPin*> TempLinkedTo = Pin->LinkedTo;
		// 	for (int32 Idx = 0; Idx < TempLinkedTo.Num(); Idx++)
		// 	{
		// 		UMGEdNode* GraphNode = Cast<UMGEdNode>(TempLinkedTo[Idx]->GetOwningNode());
		// 		if (GraphNode && BBoxTree.Children.Num() > 0)
		// 		{
		// 			AutoArrangeNodesVertically(GraphNode, BBoxTree.Children[BBoxIndex], PosX,
		// 			                           PosY + GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y *
		// 			                           2.5f);
		// 			GraphNode->DEPRECATED_NodeWidget.Pin()->MoveTo(
		// 				FVector2D(
		// 					BBoxTree.Children[BBoxIndex].SubGraphBBox.X / 2 - GraphNode->DEPRECATED_NodeWidget.Pin()->
		// 					GetDesiredSize().X / 2 + PosX, PosY), NodeFilter);
		// 			PosX += BBoxTree.Children[BBoxIndex].SubGraphBBox.X + 20;
		// 			BBoxIndex++;
		// 		}
		// 	}
		// }
	}

	void AutoArrangeNodesHorizontally(UMGEdNode* ParentNode, FNodeBoundsInfo& BBoxTree, float PosX,
	                                  float PosY)
	{
		// int32 BBoxIndex = 0;
		//
		// UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		// if (Pin)
		// {
		// 	SGraphNode::FNodeSet NodeFilter;
		// 	TArray<UEdGraphPin*> TempLinkedTo = Pin->LinkedTo;
		// 	for (int32 Idx = 0; Idx < TempLinkedTo.Num(); Idx++)
		// 	{
		// 		UMGEdNode* GraphNode = Cast<UMGEdNode>(TempLinkedTo[Idx]->GetOwningNode());
		// 		if (GraphNode && BBoxTree.Children.Num() > 0)
		// 		{
		// 			// AutoArrangeNodesVertically(GraphNode, BBoxTree.Children[BBoxIndex], PosX, PosY + GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y * 2.5f);
		// 			AutoArrangeNodesHorizontally(GraphNode, BBoxTree.Children[BBoxIndex],
		// 			                             PosX + GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X *
		// 			                             2.5f, PosY);
		//
		// 			// GraphNode->DEPRECATED_NodeWidget.Pin()->MoveTo(FVector2D(BBoxTree.Children[BBoxIndex].SubGraphBBox.X / 2 - GraphNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X / 2 + PosX, PosY), NodeFilter);
		// 			GraphNode->DEPRECATED_NodeWidget.Pin()->MoveTo(
		// 				FVector2D(
		// 					PosX, BBoxTree.Children[BBoxIndex].SubGraphBBox.Y / 2 - GraphNode->DEPRECATED_NodeWidget.
		// 					Pin()->GetDesiredSize().Y / 2 + PosY), NodeFilter);
		//
		// 			// PosX += BBoxTree.Children[BBoxIndex].SubGraphBBox.X + 20;
		// 			PosY += BBoxTree.Children[BBoxIndex].SubGraphBBox.Y + 20;
		// 			BBoxIndex++;
		// 		}
		// 	}
		// }
	}

	void GetNodeSizeInfo(UMGEdNode* ParentNode, FNodeBoundsInfo& BBoxTree,
	                     TArray<UMGEdNode*>& VisitedNodes)
	{
		BBoxTree.SubGraphBBox = ParentNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize();
		float LevelWidth = 0;
		float LevelHeight = 0;

		UEdGraphPin* Pin = FindGraphNodePin(ParentNode, EGPD_Output);
		if (Pin)
		{
			// Pin->LinkedTo.Sort(FMontageGraphCompareNodeXLocation());
			for (int32 Idx = 0; Idx < Pin->LinkedTo.Num(); Idx++)
			{
				UMGEdNode* GraphNode = Cast<UMGEdNode>(Pin->LinkedTo[Idx]->GetOwningNode());
				if (GraphNode)
				{
					const int32 ChildIdx = BBoxTree.Children.Add(FNodeBoundsInfo());
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


void UMontageEdGraph::AutoArrange(const bool bVertical)
{
	UMGEdNode* RootNode = nullptr;
	for (int32 Idx = 0; Idx < Nodes.Num(); Idx++)
	{
		RootNode = Cast<UMGEdNode_Entry>(Nodes[Idx]);
		if (RootNode)
		{
			break;
		}
	}

	if (!RootNode)
	{
		return;
	}

	MG_LOG("UMontageGraphEdGraph::AutoArrange Strategy: %s",
	         bVertical ? TEXT("Vertical") : TEXT("Horizontal"))
	const FScopedTransaction Transaction(NSLOCTEXT("MontageGraph", "MontageGraphEditorAutoArrange",
	                                               "Montage Graph Editor: Auto Arrange"));

	MGAutoArrangeHelpers::FNodeBoundsInfo BBoxTree;
	TArray<UMGEdNode*> VisitedNodes;
	MGAutoArrangeHelpers::GetNodeSizeInfo(RootNode, BBoxTree, VisitedNodes);
	//
	// SGraphNode::FNodeSet NodeFilter;
	// if (bVertical)
	// {
	// 	MGAutoArrangeHelpers::AutoArrangeNodesVertically(RootNode, BBoxTree, 0,
	// 	                                                  RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().Y *
	// 	                                                  2.5f);
	//
	// 	const float NewRootPosX = BBoxTree.SubGraphBBox.X / 2 - RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().
	// 	                                                                  X / 2;
	// 	RootNode->DEPRECATED_NodeWidget.Pin()->MoveTo(FVector2D(NewRootPosX, 0), NodeFilter);
	// }
	// else
	// {
	// 	MGAutoArrangeHelpers::AutoArrangeNodesHorizontally(RootNode, BBoxTree,
	// 	                                                    RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().X *
	// 	                                                    2.5f, 0);
	//
	// 	const float NewRootPosY = BBoxTree.SubGraphBBox.Y / 2 - RootNode->DEPRECATED_NodeWidget.Pin()->GetDesiredSize().
	// 	                                                                  Y / 2;
	// 	RootNode->DEPRECATED_NodeWidget.Pin()->MoveTo(FVector2D(0, NewRootPosY), NodeFilter);
	// }
	//
	// RootNode->DEPRECATED_NodeWidget.Pin()->GetOwnerPanel()->ZoomToFit(/*bOnlySelection=*/ false);
}

void UMontageEdGraph::GetEditingType()
{
}

void UMontageEdGraph::Clear()
{
	UMontageGraph* MontageGraphModel = GetMontageGraphModel();
	if (MontageGraphModel)
	{
		MontageGraphModel->ClearGraph();
	}

	NodeMap.Reset();
	EdgeMap.Reset();
	EntryNodes.Reset();

	for (UEdGraphNode* Node : Nodes)
	{
		if (const UMGEdNode* EdGraphNode = Cast<UMGEdNode>(Node))
		{
			UMGNode* GraphNode = EdGraphNode->RuntimeNode;
			if (GraphNode)
			{
				GraphNode->ParentNodes.Reset();
				GraphNode->ChildrenNodes.Reset();
				GraphNode->Edges.Reset();
			}
		}
	}
}

void UMontageEdGraph::SortNodes(UMGNode* RootNode)
{
	TArray<UMGNode*> CurrLevelNodes = {RootNode};
	TArray<UMGNode*> NextLevelNodes;

	while (CurrLevelNodes.Num() != 0)
	{
		for (UMGNode* Node : CurrLevelNodes)
		{
			auto Comp = [&](const UMGNode& L, const UMGNode& R)
			{
				const UMGEdNode* EdNode_LNode = NodeMap[&L];
				const UMGEdNode* EdNode_RNode = NodeMap[&R];
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
#undef LOCTEXT_NAMESPACE

int UMontageEdGraph::GetDepth(UMGNode* Root)
{
	if (!Root) return 0;
	int MaxChildDepth = 0;
	for (UMGNode* Child : Root->ChildrenNodes)
	{
		MaxChildDepth = FMath::Max(MaxChildDepth, GetDepth(Child));
	}

	if (auto MontageNode = Cast<UMGNode_Montage>(Root))
	{
		return 1 + MaxChildDepth;
	}
	
	return MaxChildDepth;
}

int UMontageEdGraph::GetBreadth(UMGNode* Root)
{
	if (!Root) return 0;

	TArray<UMGNode*> Queue;
	Queue.Add(Root);

	int MaxWidth = 0;

	while (!Queue.IsEmpty())
	{
		int LevelCount = Queue.Num();   // nodes at current level
		MaxWidth = FMath::Max(MaxWidth, LevelCount);

		for (int i = 0; i < LevelCount; i++)
		{
			UMGNode* Cur = Queue.Pop();
			for (UMGNode* Child : Cur->ChildrenNodes)
			{
				Queue.Add(Child);
			}
		}
	}
	return MaxWidth;	
}

