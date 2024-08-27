

#include "HBActionGraphSchema.h"

#include "ConnectionDrawingPolicy.h"
#include "EdGraphNode_Comment.h"
#include "HBActionGraphConnectionDrawingPolicy.h"
#include "HBActionGraphEdGraph.h"
#include "..\ActionGraphEditorLog.h"
#include "ActionGraphEditorTypes.h"
#include "ActionGraph/ActionGraphNode_Entry.h"
#include "Animation/AnimMontage.h"
#include "ActionGraph/ActionGraph.h"
#include "ActionGraph/ActionGraphEdge.h"
#include "ActionGraph/ActionGraphNode_Conduit.h"
#include "ActionGraph/ActionGraphNode_Entry.h"
#include "ActionGraph/ActionGraphNode_Action.h"
#include "Slate/SHBGraphNodeAction.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "Nodes/ActionGraphEdNode.h"
#include "Nodes/HBActionGraphEdNodeConduit.h"
#include "Nodes/HBActionGraphEdNodeEdge.h"
#include "Nodes/HBActionGraphEdNodeEntry.h"
#include "Nodes/HBActionGraphEdNodeMontage.h"

#define LOCTEXT_NAMESPACE "HBActionGraphSchema"

int32 UHBActionGraphSchema::CurrentCacheRefreshID = 0;

class FACENodeVisitorCycleChecker
{
public:
	/** Check whether a loop in the graph would be caused by linking the passed-in nodes */
	bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
	{
		VisitedNodes.Add(StartNode);
		return TraverseNodes(EndNode);
	}

private:
	TSet<UEdGraphNode*> VisitedNodes;
	TSet<UEdGraphNode*> FinishedNodes;

	bool TraverseNodes(UEdGraphNode* Node)
	{
		VisitedNodes.Add(Node);

		for (UEdGraphPin* MyPin : Node->Pins)
		{
			if (MyPin->Direction == EGPD_Output)
			{
				for (const UEdGraphPin* OtherPin : MyPin->LinkedTo)
				{
					UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
					if (VisitedNodes.Contains(OtherNode))
					{
						// Only  an issue if this is a back-edge
						return false;
					}

					if (!FinishedNodes.Contains(OtherNode) && !TraverseNodes(OtherNode))
					{
						// Only should traverse if this node hasn't been traversed
						return false;
					}
				}
			}
		}

		VisitedNodes.Remove(Node);
		FinishedNodes.Add(Node);
		return true;
	};
};

/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_AddComment

UEdGraphNode* FHBActionGraphSchemaAction_AddComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode_Comment* const CommentTemplate = NewObject<UEdGraphNode_Comment>();

	FVector2D SpawnLocation = Location;
	FSlateRect Bounds;

	const TSharedPtr<SGraphEditor> GraphEditorPtr = SGraphEditor::FindGraphEditorForGraph(ParentGraph);
	if (GraphEditorPtr.IsValid() && GraphEditorPtr->GetBoundsForSelectedNodes(/*out*/ Bounds, 50.0f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	UEdGraphNode* const NewNode = FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);

	return NewNode;
}

/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_NewNode

UEdGraphNode* FHBActionGraphSchemaAction_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, const bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewNode", "Action Graph Editor: New Node"));
		ParentGraph->Modify();
		if (FromPin)
		{
			FromPin->Modify();
		}

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		NodeTemplate->RuntimeNode->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;
	}

	return ResultNode;
}

void FHBActionGraphSchemaAction_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_NewEdge

UEdGraphNode* FHBActionGraphSchemaAction_NewEdge::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewEdge", "Action Graph Editor: New Edge"));
		ParentGraph->Modify();
		if (FromPin)
		{
			FromPin->Modify();
		}

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		NodeTemplate->RuntimeEdge->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;
	}

	return ResultNode;
}

void FHBActionGraphSchemaAction_NewEdge::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}


/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_NewNodeAction
UEdGraphNode* FHBActionGraphSchemaAction_NewNodeAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;
    
	// If there is a template, we actually use it
	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewAction", "Action Graph Editor: New Action"));
		ParentGraph->Modify();
		if (FromPin)
		{
			FromPin->Modify();
		}

		// set outer to be the graph so it doesn't go away
		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		ResultNode = NodeTemplate;

		ResultNode->SetFlags(RF_Transactional);
	}

	return ResultNode;
}

void FHBActionGraphSchemaAction_NewNodeAction::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}


/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_NewNodeEntry


UEdGraphNode* FHBActionGraphSchemaAction_NewEntryNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;
    
	// If there is a template, we actually use it
	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("New Entry Node", "Action Graph Editor: New Entry"));
		ParentGraph->Modify();
		if (FromPin)
		{
			FromPin->Modify();
		}

		// set outer to be the graph so it doesn't go away
		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		ResultNode = NodeTemplate;

		ResultNode->SetFlags(RF_Transactional);
	}

	return ResultNode;
}


void FHBActionGraphSchemaAction_NewEntryNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_NewNodeConduit

UEdGraphNode* FHBActionGraphSchemaAction_NewNodeConduit::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	// If there is a template, we actually use it
	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewConduit", "Action Graph Editor: New Conduit"));
		ParentGraph->Modify();
		if (FromPin)
		{
			FromPin->Modify();
		}

		// set outer to be the graph so it doesn't go away
		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		ResultNode = NodeTemplate;

		ResultNode->SetFlags(RF_Transactional);
	}

	return ResultNode;
}

void FHBActionGraphSchemaAction_NewNodeConduit::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}



/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_AutoArrangeVertical

UEdGraphNode* FHBActionGraphSchemaAction_AutoArrangeVertical::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UHBActionGraphEdGraph* Graph = Cast<UHBActionGraphEdGraph>(ParentGraph);
	if (Graph)
	{
		ACTIONGRAPH_LOG(Verbose, TEXT("FHBActionGraphSchemaAction_AutoArrangeVertical::PerformAction"))
		Graph->AutoArrange(true);
	}

	return nullptr;
}

/////////////////////////////////////////////////////
// FHBActionGraphSchemaAction_AutoArrangeHorizontal

UEdGraphNode* FHBActionGraphSchemaAction_AutoArrangeHorizontal::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UHBActionGraphEdGraph* Graph = Cast<UHBActionGraphEdGraph>(ParentGraph);
	if (Graph)
	{
		ACTIONGRAPH_LOG(Verbose, TEXT("FHBActionGraphSchemaAction_AutoArrangeVertical::PerformAction Horizontal"))
		Graph->AutoArrange(false);
	}

	return nullptr;
}


/////////////////////////////////////////////////////
// UHBActionGraphSchema

void UHBActionGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	UActionGraph* HBActionGraph = Cast<UActionGraph>(Graph.GetOuter());
	ACTIONGRAPH_LOG(Verbose, TEXT("CreateDefaultNodesForGraph - Graph, Outer HBActionGraph: %s"), *GetNameSafe(HBActionGraph))

	// Create the entry/exit tunnels
	FGraphNodeCreator<UActionGraphEdNodeEntry> NodeCreator(Graph);
	UActionGraphEdNodeEntry* EntryNode = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	SetNodeMetaData(EntryNode, FNodeMetadata::DefaultGraphNode);

	// // Create runtime node for this editor node. Entry nodes gets a bare bone anim base one as well with blank anim related info.
	EntryNode->RuntimeNode = NewObject<UActionGraphNode_Entry>(HBActionGraph, UActionGraphNode_Entry::StaticClass());
	
	if (UHBActionGraphEdGraph* EdHBActionGraph = CastChecked<UHBActionGraphEdGraph>(&Graph))
	{
		EdHBActionGraph->EntryNodes.Add(EntryNode);
	}
}

EGraphType UHBActionGraphSchema::GetGraphType(const UEdGraph* TestEdGraph) const
{
	return GT_StateMachine;
}

void UHBActionGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	Super::GetGraphContextActions(ContextMenuBuilder);
	const bool bHasNoParent = (ContextMenuBuilder.FromPin == nullptr);

	// First build up non anim related actions
	if (bHasNoParent)
	{
		const TSharedPtr<FHBActionGraphSchemaAction_AddComment> Action_AddComment(
			new FHBActionGraphSchemaAction_AddComment(
			LOCTEXT("AddComent", "Add Comment"),
			LOCTEXT("AddComent_ToolTip", "Adds comment")
		));
		
		const TSharedPtr<FHBActionGraphSchemaAction_AutoArrangeVertical> ActionAutoArrangeVertical(new FHBActionGraphSchemaAction_AutoArrangeVertical(
			LOCTEXT("AutoArrangeNodeCategory", "Auto Arrange"),
			LOCTEXT("AutoArrangeVertical", "Auto Arrange (Vertically)"),
			LOCTEXT("AutoArrangeVertically_ToolTip", "Re-arrange graph layout vertically (from top to bottom). Similar to Behavior Trees."),
			0
		));

		const TSharedPtr<FHBActionGraphSchemaAction_AutoArrangeHorizontal> ActionAutoArrangeHorizontal(new FHBActionGraphSchemaAction_AutoArrangeHorizontal(
			LOCTEXT("AutoArrangeNodeCategory", "Auto Arrange"),
			LOCTEXT("AutoArrangeHorizontal", "Auto Arrange (Horizontally)"),
			LOCTEXT("AutoArrangeHorizontally_ToolTip", "Re-arrange graph layout horizontally (from left to right). This is far from perfect, but still handy. Will be revisited and improved upon in a future update."),
			0
		));

		ContextMenuBuilder.AddAction(Action_AddComment);
		ContextMenuBuilder.AddAction(ActionAutoArrangeVertical);
		ContextMenuBuilder.AddAction(ActionAutoArrangeHorizontal);
	}

	const TSharedPtr<FHBActionGraphSchemaAction_NewEntryNode> EntryAction(new FHBActionGraphSchemaAction_NewEntryNode(
		LOCTEXT("ActionGraphNode", "Action Graph Node"),
		LOCTEXT("AddEntry", "Add Entry..."),
		LOCTEXT("AddEntry",
		        "A new Entry state (suitable to use to branch out HBActions after entry point based on an initial Transition Input)"),
		1
	));
	EntryAction->NodeTemplate = NewObject<UActionGraphEdNodeEntry>(ContextMenuBuilder.OwnerOfTemporaries);
	EntryAction->NodeTemplate->RuntimeNode = NewObject<UActionGraphNode_Entry>(EntryAction->NodeTemplate);
	ContextMenuBuilder.AddAction(EntryAction);

	const TSharedPtr<FHBActionGraphSchemaAction_NewNodeConduit> ConduitAction(new FHBActionGraphSchemaAction_NewNodeConduit(
		LOCTEXT("ActionGraphNode", "Action Graph Node"),
		LOCTEXT("AddConduit", "Add Conduit..."),
		LOCTEXT("AddConduit",
		        "A new conduit state (suitable to use to branch out HBActions after entry point based on an initial Transition Input)"),
		1
	));
	ConduitAction->NodeTemplate = NewObject<UActionGraphEdNodeConduit>(ContextMenuBuilder.OwnerOfTemporaries);
	ConduitAction->NodeTemplate->RuntimeNode = NewObject<UActionGraphNode_Conduit>(ConduitAction->NodeTemplate);
	ContextMenuBuilder.AddAction(ConduitAction);

	
	const TSharedPtr<FHBActionGraphSchemaAction_NewNodeAction> ActionNodeAction(
		new FHBActionGraphSchemaAction_NewNodeAction(
			LOCTEXT("ActionGraphNode", "Action Graph Node"),
			LOCTEXT("AddAction", "Add Action..."),
			LOCTEXT("AddAction",
			        "A "),
			1
		));
	ActionNodeAction->NodeTemplate = NewObject<UActionGraphEdNodeMontage>(ContextMenuBuilder.OwnerOfTemporaries);
	ActionNodeAction->NodeTemplate->RuntimeNode = NewObject<UActionGraphNode_Action>(ActionNodeAction->NodeTemplate);
	ContextMenuBuilder.AddAction(ActionNodeAction);


	UActionGraph* HBActionGraph = CastChecked<UActionGraph>(ContextMenuBuilder.CurrentGraph->GetOuter());
	// Now, build up actions for anim nodes
	if (!HBActionGraph || HBActionGraph->NodeType == nullptr)
	{
		return;
	}

	TSet<TSubclassOf<UActionGraphNode>> Visited;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(HBActionGraph->NodeType) && !It->HasAnyClassFlags(CLASS_Abstract) && !Visited.Contains(*It))
		{
			TSubclassOf<UActionGraphNode> NodeType = *It;
			if (It->GetName().StartsWith("REINST") || It->GetName().StartsWith("SKEL"))
			{
				continue;
			}

			if (NodeType.GetDefaultObject()->ContextMenuName.IsEmpty())
			{
				continue;
			}

			ACTIONGRAPH_LOG(Verbose, TEXT("GetGraphContextActions - Create action from %s"), *NodeType->GetName())
			CreateAndAddActionToContextMenu(ContextMenuBuilder, NodeType);
			Visited.Add(NodeType);
		}
	}
}

void UHBActionGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetContextMenuActions(Menu, Context);
}

FLinearColor UHBActionGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	// light grey
	return FLinearColor(1.f, 0.101961f, 0.101961f, 1.0f);
}

// ReSharper disable once CppConstValueFunctionReturnType
const FPinConnectionResponse UHBActionGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Either one or the other is invalid
	if (!PinA || !PinB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinInvalid", "Invalid pins"));
	}

	UEdGraphNode* OwningNodeA = PinA->GetOwningNode();
	UEdGraphNode* OwningNodeB = PinB->GetOwningNode();

	// Either one or the other is invalid
	if (!OwningNodeA || !OwningNodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinInvalidNodes", "Invalid nodes"));
	}

	// Make sure the pins are not on the same node
	if (OwningNodeA == OwningNodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode", "Can't connect node to itself"));
	}


	UActionGraphEdNode* HBActionGraphNodeIn = Cast<UActionGraphEdNode>(OwningNodeA);
	UActionGraphEdNode* HBActionGraphNodeOut = Cast<UActionGraphEdNode>(OwningNodeB);

	if(!HBActionGraphNodeOut->RuntimeNode)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid HBActionGraph Node"));
	}


	// Allow wiring from Entry node to Conduit Node
	if (OwningNodeA->IsA(UActionGraphEdNodeEntry::StaticClass()) && OwningNodeB->IsA(UActionGraphEdNodeConduit::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectEntryToConduit", "Connect entry node to conduit."));
	}

	// Allow wiring from Conduit node to HBAction nodes
	if (OwningNodeA->IsA(UActionGraphEdNodeConduit::StaticClass()) && HBActionGraphNodeOut)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnectEntryToConduit", "Connect conduit node to HBAction node"));
	}

	// Disallow wiring onto an entry node
	if (OwningNodeB->IsA(UActionGraphEdNodeEntry::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorToEntryNode", "Cannot connect to an entry node (can only create connections **from** entry node)"));
	}

	if (HBActionGraphNodeOut == nullptr || HBActionGraphNodeIn == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid HBActionGraph Node"));
	}

	// check for cycles
	FACENodeVisitorCycleChecker CycleChecker;
	if (!CycleChecker.CheckForLoop(OwningNodeA, OwningNodeB))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	FText ErrorMessage;
	if (!HBActionGraphNodeIn->RuntimeNode->CanCreateConnectionTo(HBActionGraphNodeOut->RuntimeNode, HBActionGraphNodeIn->GetOutputPin()->LinkedTo.Num(), ErrorMessage))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}

	if (!HBActionGraphNodeOut->RuntimeNode->CanCreateConnectionFrom(HBActionGraphNodeIn->RuntimeNode, HBActionGraphNodeOut->GetInputPin()->LinkedTo.Num(), ErrorMessage))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes with Transition Input"));

}

bool UHBActionGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	// return Super::TryCreateConnection(A, B);

	// We don't actually care about the pin, we want the node that is being dragged between
	UActionGraphEdNode* NodeA = Cast<UActionGraphEdNode>(A->GetOwningNode());
	UActionGraphEdNode* NodeB = Cast<UActionGraphEdNode>(B->GetOwningNode());

	// Check that this edge doesn't already exist
	for (const UEdGraphPin* TestPin : A->LinkedTo)
	{
		UEdGraphNode* ChildNode = TestPin->GetOwningNode();
		if (const UActionGraphEdNodeEdge* EdNode_Edge = Cast<UActionGraphEdNodeEdge>(ChildNode))
		{
			ChildNode = EdNode_Edge->GetEndNode();
		}

		if (ChildNode == NodeB)
		{
			return false;
		}
	}

	if (NodeA && NodeB)
	{
		const bool bModified = Super::TryCreateConnection(A, B);
		return bModified;
	}

	return false;
}

bool UHBActionGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const
{
	UActionGraphEdNode* NodeA = Cast<UActionGraphEdNode>(A->GetOwningNode());
	UActionGraphEdNode* NodeB = Cast<UActionGraphEdNode>(B->GetOwningNode());

	// Handle conversion from conduit to HBActionGraph Nodes
	if (const UActionGraphEdNodeConduit* ConduitNode = Cast<UActionGraphEdNodeConduit>(A->GetOwningNode()))
	{
		if (NodeB && NodeB->GetInputPin())
		{
			ACTIONGRAPH_LOG(Verbose, TEXT("CreateAutomaticConversionNodeAndConnections for conduit"));
			CreateEdgeConnection(A, B, ConduitNode, NodeB);
			return true;
		}
	}

	// Are nodes and pins all valid?
	if (!NodeA || !NodeA->GetOutputPin() || !NodeB || !NodeB->GetInputPin())
	{
		ACTIONGRAPH_LOG(Verbose, TEXT("CreateAutomaticConversionNodeAndConnections failed"));
		return false;
	}

	ACTIONGRAPH_LOG(Verbose, TEXT("CreateAutomaticConversionNodeAndConnections ok"));
	CreateEdgeConnection(A, B, NodeA, NodeB);

	return true;
}

FConnectionDrawingPolicy* UHBActionGraphSchema::CreateConnectionDrawingPolicy(const int32 InBackLayerID, const int32 InFrontLayerID, const float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FHBActionGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

void UHBActionGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakNodeLinks", "Break Node Links"));
	Super::BreakNodeLinks(TargetNode);

	// UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(&TargetNode);
	// FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

void UHBActionGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, const bool bSendsNodeNotification) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));
	Super::BreakPinLinks(TargetPin, bSendsNodeNotification);
	// UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(TargetPin.GetOwningNode());
	// FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

void UHBActionGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));
	Super::BreakSinglePinLink(SourcePin, TargetPin);

	// UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(TargetPin->GetOwningNode());
	// FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

UEdGraphPin* UHBActionGraphSchema::DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePin, EEdGraphPinDirection InSourcePinDirection) const
{
	const UActionGraphEdNode* EdGraphNode = Cast<UActionGraphEdNode>(InTargetNode);
	switch (InSourcePinDirection)
	{
	case EGPD_Input:
		return EdGraphNode->GetOutputPin();
	case EGPD_Output:
		return EdGraphNode->GetInputPin();
	default:
		return nullptr;
	}
}

bool UHBActionGraphSchema::SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const
{
	return Cast<UActionGraphEdNode>(InTargetNode) != nullptr;
}

bool UHBActionGraphSchema::IsCacheVisualizationOutOfDate(const int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UHBActionGraphSchema::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UHBActionGraphSchema::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

void UHBActionGraphSchema::GetGraphDisplayInformation(const UEdGraph& Graph, FGraphDisplayInfo& DisplayInfo) const
{
	Super::GetGraphDisplayInformation(Graph, DisplayInfo);
	DisplayInfo.Tooltip = LOCTEXT("DisplayInfo_Tooltip", "HBActionGraph");
}

void UHBActionGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const
{
	Super::DroppedAssetsOnGraph(Assets, GraphPosition, Graph);
	if (Graph != nullptr)
	{
		if (UAnimationAsset* AnimationAsset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets))
		{
			SpawnNodeFromAsset(AnimationAsset, GraphPosition, Graph, nullptr);
		}
	}
}

void UHBActionGraphSchema::DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const
{
	Super::DroppedAssetsOnNode(Assets, GraphPosition, Node);

	UAnimationAsset* Asset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);
	UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(Node);
	if (Asset && GraphNode && GraphNode->RuntimeNode)
	{
		const FScopedTransaction Transaction(LOCTEXT("UpdateNodeWithAsset", "Updating Node with Asset"));
		GraphNode->Modify();
		// GraphNode->RuntimeNode->SetAnimationAsset(Asset);
		GraphNode->GetSchema()->ForceVisualizationCacheClear();
		GraphNode->ReconstructNode();
	}
}

void UHBActionGraphSchema::DroppedAssetsOnPin(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphPin* Pin) const
{
	UAnimationAsset* Asset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);

	if (Asset && Pin)
	{
		FVector2D FixedOffset = GetFixedOffsetFromPin(Pin);

		// if (const UActionGraphEdNode* EdGraphNode = Cast<UActionGraphEdNode>(Pin->GetOwningNode()))
		// {
		// 	if (EdGraphNode->SlateNode)
		// 	{
		// 		const FVector2D Size = EdGraphNode->SlateNode->ComputeDesiredSize(FSlateApplication::Get().GetApplicationScale());
		// 		FixedOffset.X = Size.X + 100.f;
		// 	}
		// }

		SpawnNodeFromAsset(Asset, GraphPosition + FixedOffset, Pin->GetOwningNode()->GetGraph(), Pin);
	}
}

void UHBActionGraphSchema::GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& bOutOkIcon) const
{
	Super::GetAssetsGraphHoverMessage(Assets, HoverGraph, OutTooltipText, bOutOkIcon);

	UAnimationAsset* AnimationAsset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);
	if (AnimationAsset)
	{
		if (UAnimMontage* Montage = FAssetData::GetFirstAsset<UAnimMontage>(Assets))
		{
			bOutOkIcon = true;
			OutTooltipText = LOCTEXT("MontagesInGraphs", "Drop Montage here to create a new node in the HBAction graph").ToString();
		}
		else if (UAnimSequence* Sequence = FAssetData::GetFirstAsset<UAnimSequence>(Assets))
		{
			bOutOkIcon = true;
			OutTooltipText = LOCTEXT("SequencesInGraphs", "Drop Sequence here to create a new node in the HBAction graph. Montage will be created dynamically from the Anim Sequence.").ToString();
		}
	}
}

void UHBActionGraphSchema::GetAssetsPinHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphPin* HoverPin, FString& OutTooltipText, bool& bOutOkIcon) const
{
	Super::GetAssetsPinHoverMessage(Assets, HoverPin, OutTooltipText, bOutOkIcon);

	UAnimationAsset* Asset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);
	if (!Asset || !HoverPin)
	{
		OutTooltipText = TEXT("");
		bOutOkIcon = false;
		return;
	}

	// const bool bDirectionMatch = HoverPin->Direction == EGPD_Input;
	const bool bDirectionMatch = IsHoverPinMatching(HoverPin);

	if (bDirectionMatch)
	{
		UEdGraphNode* OwningNode = HoverPin->GetOwningNode();
		if (UAnimMontage* Montage = Cast<UAnimMontage>(Asset))
		{
			bOutOkIcon = true;
			OutTooltipText = FText::Format(LOCTEXT("MontagesOnPin", "Connect Montage {0} in the HBAction graph to {1}"),
				FText::FromString(Asset->GetName()),
				OwningNode ? OwningNode->GetNodeTitle(ENodeTitleType::ListView) : FText::FromName(HoverPin->PinName)
			).ToString();
		}
		else if (UAnimSequence* Sequence = Cast<UAnimSequence>(Asset))
		{
			bOutOkIcon = true;
			OutTooltipText = FText::Format(LOCTEXT("SequencesOnPin", "Connect Sequence {0} in the HBAction graph to {1}"),
				FText::FromString(Asset->GetName()),
				OwningNode ? OwningNode->GetNodeTitle(ENodeTitleType::ListView) : FText::FromName(HoverPin->PinName)
			).ToString();
		}
	}
	else
	{
		bOutOkIcon = false;
		OutTooltipText = LOCTEXT("AssetPinHoverMessage_Fail", "Type or direction mismatch; must be wired to a HBAction input").ToString();
	}
}

void UHBActionGraphSchema::GetAssetsNodeHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphNode* HoverNode, FString& OutTooltipText, bool& bOutOkIcon) const
{
	Super::GetAssetsNodeHoverMessage(Assets, HoverNode, OutTooltipText, bOutOkIcon);

	UAnimationAsset* Asset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);
	if ((Asset == nullptr) || (HoverNode == nullptr) || !HoverNode->IsA(UEdGraphNode::StaticClass()))
	{
		OutTooltipText = TEXT("");
		bOutOkIcon = false;
		return;
	}

	const bool bCanPlayAsset = SupportNodeClassForAsset(Asset->GetClass(), HoverNode);
	if (bCanPlayAsset)
	{
		bOutOkIcon = true;
		OutTooltipText = FText::Format(LOCTEXT("AssetNodeHoverMessage_Success", "Change node to play '{0}'"), FText::FromString(Asset->GetName())).ToString();
	}
	else
	{
		bOutOkIcon = false;
		OutTooltipText = FText::Format(LOCTEXT("AssetNodeHoverMessage_Fail", "Cannot play '{0}' on this node type"),  FText::FromString(Asset->GetName())).ToString();
	}
}

bool UHBActionGraphSchema::CanDuplicateGraph(UEdGraph* InSourceGraph) const
{
	return false;
}

int32 UHBActionGraphSchema::GetNodeSelectionCount(const UEdGraph* Graph) const
{
	if (Graph)
	{
		const TSharedPtr<SGraphEditor> GraphEditorPtr = SGraphEditor::FindGraphEditorForGraph(Graph);
		if (GraphEditorPtr.IsValid())
		{
			return GraphEditorPtr->GetNumberOfSelectedNodes();
		}
	}

	return 0;
}

TSharedPtr<FEdGraphSchemaAction> UHBActionGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FHBActionGraphSchemaAction_AddComment));
}

void UHBActionGraphSchema::SpawnNodeFromAsset(UAnimationAsset* Asset, const FVector2D& GraphPosition, UEdGraph* Graph, UEdGraphPin* PinIfAvailable)
{
	check(Graph);
	check(Graph->GetSchema()->IsA(UHBActionGraphSchema::StaticClass()));
	check(Asset);

	const bool bDirectionMatch = (PinIfAvailable == nullptr) || IsHoverPinMatching(PinIfAvailable);
	if (bDirectionMatch)
	{
		UClass* NewNodeClass = GetNodeClassForAnimAsset(Asset, Graph);
		UClass* NewRuntimeClass = GetRuntimeClassForAnimAsset(Asset, Graph);
		if (NewNodeClass && NewRuntimeClass)
		{
			ACTIONGRAPH_LOG(Verbose, TEXT("SpawnNodeFromAsset - NewNodeClass: %s, NewRuntimeClass: %s"), *NewNodeClass->GetName(), *NewRuntimeClass->GetName());
			UActionGraphEdNode* NewNode = NewObject<UActionGraphEdNode>(Graph, NewNodeClass);
			NewNode->RuntimeNode = NewObject<UActionGraphNode>(NewNode, NewRuntimeClass);
			// NewNode->RuntimeNode->SetAnimationAsset(Asset);

			FHBActionGraphSchemaAction_NewNode Action;
			Action.NodeTemplate = NewNode;
			Action.PerformAction(Graph, PinIfAvailable, GraphPosition);
		}
	}
}

UClass* UHBActionGraphSchema::GetNodeClassForAnimAsset(const UAnimationAsset* Asset, const UEdGraph* Graph)
{
	if (Asset->GetClass()->IsChildOf(UAnimMontage::StaticClass()))
	{
		return UActionGraphEdNode::StaticClass();
	}

	if (Asset->GetClass()->IsChildOf(UAnimSequence::StaticClass()))
	{
		return UActionGraphEdNode::StaticClass();
	}

	return nullptr;
}

UClass* UHBActionGraphSchema::GetRuntimeClassForAnimAsset(const UAnimationAsset* Asset, const UEdGraph* Graph)
{
	UActionGraph* HBActionGraph = Cast<UActionGraph>(Graph->GetOuter());
	//
	// if (Asset->GetClass()->IsChildOf(UAnimMontage::StaticClass()))
	// {
	// 	return HBActionGraph ? HBActionGraph->DefaultNodeMontageType : UHBActionGraphNodeMontage::StaticClass();
	// }
	//
	// if (Asset->GetClass()->IsChildOf(UAnimSequence::StaticClass()))
	// {
	// 	return HBActionGraph ? HBActionGraph->DefaultNodeSequenceType : UHBActionGraphNodeSequence::StaticClass();
	// }

	return nullptr;
}

bool UHBActionGraphSchema::SupportNodeClassForAsset(UClass* AssetClass, const UEdGraphNode* InGraphNode)
{
	const UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(InGraphNode);
	if (GraphNode && GraphNode->RuntimeNode)
	{
		// See if this node supports this asset type (primary or not)
		// return GraphNode->RuntimeNode->SupportsAssetClass(AssetClass);
	}

	return false;
}

void UHBActionGraphSchema::CreateEdgeConnection(UEdGraphPin* PinA, UEdGraphPin* PinB, const UActionGraphEdNode* OwningNodeA, const UActionGraphEdNode* OwningNodeB) const
{
	const FVector2D InitPos((OwningNodeA->NodePosX + OwningNodeB->NodePosX) / 2, (OwningNodeA->NodePosY + OwningNodeB->NodePosY) / 2);

	// UInputAction* DefaultInput = nullptr;
	// if (const UHBActionGraphEdGraph* HBActionGraph = Cast<UHBActionGraphEdGraph>(OwningNodeA->GetGraph()))
	// {
	// 	if (const UHBActionGraph* HBActionGraphModel = HBActionGraph->GetHBActionGraphModel())
	// 	{
	// 		DefaultInput = HBActionGraphModel->DefaultInputAction;
	// 	}
	// }

	FHBActionGraphSchemaAction_NewEdge Action;
	Action.NodeTemplate = NewObject<UActionGraphEdNodeEdge>(OwningNodeA->GetGraph());

	UActionGraphEdge* RuntimeEdge = NewObject<UActionGraphEdge>(Action.NodeTemplate, UActionGraphEdge::StaticClass());
	// RuntimeEdge->TransitionInput = DefaultInput;
	Action.NodeTemplate->SetRuntimeEdge(RuntimeEdge);

	Action.PerformAction(OwningNodeA->GetGraph(), PinB, InitPos, true);
	// Always create connections from node A to B, don't allow adding in reverse
	Action.NodeTemplate->CreateConnections(OwningNodeA, OwningNodeB);
	
}

void UHBActionGraphSchema::CreateAndAddActionToContextMenu(FGraphContextMenuBuilder& ContextMenuBuilder, const TSubclassOf<UActionGraphNode> NodeType) const
{
	if (!NodeType)
	{
		return;
	}


	UActionGraphNode* NodeCDO = NodeType.GetDefaultObject();
	if (!NodeCDO)
	{
		return;
	}

	const FText AddToolTip = LOCTEXT("NewCombatGraphNodeTooltip", "Add node here");

	FText Description = NodeCDO->ContextMenuName;
	FString NodeName = NodeType->GetName();
	NodeName.RemoveFromEnd("_C");

	// Shouldn't happen. ContextMenuName if empty will prevent execution of CreateAndAddActionToContextMenu
	if (Description.IsEmpty())
	{
		Description = FText::FromString(NodeName);
	}

	// If BP node type, set category to something specific (can be further defined if Context Menu Name, with a "|" which will override the category)
	FText Category = NodeType->IsNative() ? LOCTEXT("CombatGraphNodeAction", "Action Graph Node") : FText::FromString(TEXT("Custom"));

	// Split up ContextMenuName by "|" and create top category if there is more than one level
	const FString ContextMenu = NodeCDO->ContextMenuName.ToString();
	FString Left, Right;
	if (ContextMenu.Split(TEXT("|"), &Left, &Right))
	{
		Category = FText::FromString(Left.TrimStartAndEnd());
		Description = FText::FromString(Right.TrimStartAndEnd());
	}

	// If BP node, append class name
	if (!NodeType->IsNative() && NodeCDO->bIncludeClassNameInContextMenu)
	{
		Description = FText::FromString(FString::Printf(TEXT("%s (%s)"), *Description.ToString(), *NodeName));
	}

	const TSharedPtr<FHBActionGraphSchemaAction_NewNode> Action(new FHBActionGraphSchemaAction_NewNode(Category, Description, AddToolTip, 1));
	Action->NodeTemplate = NewObject<UActionGraphEdNode>(ContextMenuBuilder.OwnerOfTemporaries, "Template");
	Action->NodeTemplate->RuntimeNode = NewObject<UActionGraphNode>(Action->NodeTemplate, NodeType);
	ContextMenuBuilder.AddAction(Action);
}

bool UHBActionGraphSchema::IsHoverPinMatching(const UEdGraphPin* InHoverPin)
{
	check(InHoverPin);

	const bool bIsValidName = InHoverPin->PinName == UActionGraphPinNames::PinName_In ||
		InHoverPin->PinName == UActionGraphPinNames::PinName_Out;

	return (InHoverPin->Direction == EGPD_Input || InHoverPin->Direction == EGPD_Output) && bIsValidName;
}

FVector2D UHBActionGraphSchema::GetFixedOffsetFromPin(const UEdGraphPin* InHoverPin)
{
	// If don't have access to bounding information for node, using fixed offset that should work for most cases.
	// Start at 450.f, and is further refined based on SlateNode bounding info is available
	FVector2D FixedOffset(450.0f, 0.f);
	FVector2D DesiredSize(0.f, 0.f);

	if (const UActionGraphEdNode* EdGraphNode = Cast<UActionGraphEdNode>(InHoverPin->GetOwningNode()))
	{
		if (EdGraphNode->SlateNode)
		{
			DesiredSize = EdGraphNode->SlateNode->ComputeDesiredSize(FSlateApplication::Get().GetApplicationScale());
		}
	}

	// if (InHoverPin->PinName == UActionGraphPinNames::PinName_In)
	// {
	// 	FixedOffset = FVector2D(-(DesiredSize.X * 2 + 100.f), -DesiredSize.Y);
	// }
	// else if (InHoverPin->PinName == UActionGraphPinNames::PinName_In)
	// {
	// 	FixedOffset = FVector2D(-DesiredSize.X, -(DesiredSize.Y * 2 + 100.f));
	// }
	// else if (InHoverPin->PinName == UActionGraphPinNames::PinName_Out)
	// {
	// 	FixedOffset = FVector2D(100.f, -DesiredSize.Y);
	// }
	// else if (InHoverPin->PinName == UActionGraphPinNames::PinName_Out)
	// {
	// 	FixedOffset = FVector2D(-DesiredSize.X, 100.f);
	// }


	return DesiredSize + FixedOffset;
}

#undef LOCTEXT_NAMESPACE
