

#include "MontageGraphSchema.h"

#include "ConnectionDrawingPolicy.h"
#include "EdGraphNode_Comment.h"
#include "MontageGraphConnectionDrawingPolicy.h"
#include "MontageGraphEditorLog.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/MontageGraphNode_Entry.h"
#include "Animation/AnimMontage.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphEdge.h"
#include "MontageGraph/MontageGraphNode_Selector.h"
#include "MontageGraph/MontageGraphNode_Animation.h"
#include "Slate/SGraphNodeAction.h"

#include "Nodes/MontageGraphEdNode.h"
#include "Nodes/MontageGraphEdNodeSelector.h"
#include "Nodes\MontageGraphEdNodeEdge.h"
#include "Nodes\MontageGraphEdNodeEntry.h"
#include "Nodes/MontageGraphEdNodeMontage.h"

#define LOCTEXT_NAMESPACE "MontageGraphSchema"

int32 UMontageGraphSchema::CurrentCacheRefreshID = 0;

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
// FMontageGraphSchemaAction_AddComment

UEdGraphNode* FMontageGraphSchemaAction_AddComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
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
// FMontageGraphSchemaAction_NewNode

UEdGraphNode* FMontageGraphSchemaAction_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, const bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewNode", "Montage Graph Editor: New Node"));
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

void FMontageGraphSchemaAction_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

/////////////////////////////////////////////////////
// FMontageGraphSchemaAction_NewEdge

UEdGraphNode* FMontageGraphSchemaAction_NewEdge::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewEdge", "Montage Graph Editor: New Edge"));
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

void FMontageGraphSchemaAction_NewEdge::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}


/////////////////////////////////////////////////////
// FMontageGraphSchemaAction_NewNodeAction
UEdGraphNode* FMontageGraphSchemaAction_NewNodeAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;
    
	// If there is a template, we actually use it
	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewAction", "Montage Graph Editor: New Action"));
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

void FMontageGraphSchemaAction_NewNodeAction::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}


/////////////////////////////////////////////////////
// FMontageGraphSchemaAction_NewNodeEntry


UEdGraphNode* FMontageGraphSchemaAction_NewEntryNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;
    
	// If there is a template, we actually use it
	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("New Entry Node", "Montage Graph Editor: New Entry"));
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


void FMontageGraphSchemaAction_NewEntryNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

/////////////////////////////////////////////////////
// FMontageGraphSchemaAction_NewNodeSelector

UEdGraphNode* FMontageGraphSchemaAction_NewNodeSelector::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UEdGraphNode* ResultNode = nullptr;

	// If there is a template, we actually use it
	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("CombatGraphEditorNewSelector", "Montage Graph Editor: New Selector"));
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

void FMontageGraphSchemaAction_NewNodeSelector::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}



/////////////////////////////////////////////////////
// FMontageGraphSchemaAction_AutoArrangeVertical

UEdGraphNode* FMontageGraphSchemaAction_AutoArrangeVertical::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UMontageGraphEdGraph* Graph = Cast<UMontageGraphEdGraph>(ParentGraph);
	if (Graph)
	{
		MG_ERROR(Verbose, TEXT("FMontageGraphSchemaAction_AutoArrangeVertical::PerformAction"))
		Graph->AutoArrange(true);
	}

	return nullptr;
}

/////////////////////////////////////////////////////
// FMontageGraphSchemaAction_AutoArrangeHorizontal

UEdGraphNode* FMontageGraphSchemaAction_AutoArrangeHorizontal::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UMontageGraphEdGraph* Graph = Cast<UMontageGraphEdGraph>(ParentGraph);
	if (Graph)
	{
		MG_ERROR(Verbose, TEXT("FMontageGraphSchemaAction_AutoArrangeVertical::PerformAction Horizontal"))
		Graph->AutoArrange(false);
	}

	return nullptr;
}


/////////////////////////////////////////////////////
// UMontageGraphSchema

void UMontageGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	UMontageGraph* MontageGraph = Cast<UMontageGraph>(Graph.GetOuter());
	MG_ERROR(Verbose, TEXT("CreateDefaultNodesForGraph - Graph, Outer MontageGraph: %s"), *GetNameSafe(MontageGraph))

	// Create the entry/exit tunnels
	FGraphNodeCreator<UMontageGraphEdNodeEntry> NodeCreator(Graph);
	UMontageGraphEdNodeEntry* EntryNode = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	SetNodeMetaData(EntryNode, FNodeMetadata::DefaultGraphNode);

	// // Create runtime node for this editor node. Entry nodes gets a bare bone anim base one as well with blank anim related info.
	EntryNode->RuntimeNode = NewObject<UMontageGraphNode_Entry>(MontageGraph, UMontageGraphNode_Entry::StaticClass());
	
	if (UMontageGraphEdGraph* EdMontageGraph = CastChecked<UMontageGraphEdGraph>(&Graph))
	{
		EdMontageGraph->EntryNodes.Add(EntryNode);
	}
}

EGraphType UMontageGraphSchema::GetGraphType(const UEdGraph* TestEdGraph) const
{
	return GT_StateMachine;
}

void UMontageGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	Super::GetGraphContextActions(ContextMenuBuilder);
	const bool bHasNoParent = (ContextMenuBuilder.FromPin == nullptr);

	// First build up non anim related actions
	if (bHasNoParent)
	{
		const TSharedPtr<FMontageGraphSchemaAction_AddComment> Action_AddComment(
			new FMontageGraphSchemaAction_AddComment(
			LOCTEXT("AddComent", "Add Comment"),
			LOCTEXT("AddComent_ToolTip", "Adds comment")
		));
		
		const TSharedPtr<FMontageGraphSchemaAction_AutoArrangeVertical> ActionAutoArrangeVertical(new FMontageGraphSchemaAction_AutoArrangeVertical(
			LOCTEXT("AutoArrangeNodeCategory", "Auto Arrange"),
			LOCTEXT("AutoArrangeVertical", "Auto Arrange (Vertically)"),
			LOCTEXT("AutoArrangeVertically_ToolTip", "Re-arrange graph layout vertically (from top to bottom). Similar to Behavior Trees."),
			0
		));

		const TSharedPtr<FMontageGraphSchemaAction_AutoArrangeHorizontal> ActionAutoArrangeHorizontal(new FMontageGraphSchemaAction_AutoArrangeHorizontal(
			LOCTEXT("AutoArrangeNodeCategory", "Auto Arrange"),
			LOCTEXT("AutoArrangeHorizontal", "Auto Arrange (Horizontally)"),
			LOCTEXT("AutoArrangeHorizontally_ToolTip", "Re-arrange graph layout horizontally (from left to right). This is far from perfect, but still handy. Will be revisited and improved upon in a future update."),
			0
		));

		ContextMenuBuilder.AddAction(Action_AddComment);
		ContextMenuBuilder.AddAction(ActionAutoArrangeVertical);
		ContextMenuBuilder.AddAction(ActionAutoArrangeHorizontal);
	}

	const TSharedPtr<FMontageGraphSchemaAction_NewEntryNode> EntryAction(new FMontageGraphSchemaAction_NewEntryNode(
		LOCTEXT("MontageGraphNode", "Montage Graph Node"),
		LOCTEXT("AddEntry", "Add Entry..."),
		LOCTEXT("AddEntry",
		        "A new Entry state (suitable to use to branch out HBActions after entry point based on an initial Transition Input)"),
		1
	));
	EntryAction->NodeTemplate = NewObject<UMontageGraphEdNodeEntry>(ContextMenuBuilder.OwnerOfTemporaries);
	EntryAction->NodeTemplate->RuntimeNode = NewObject<UMontageGraphNode_Entry>(EntryAction->NodeTemplate);
	ContextMenuBuilder.AddAction(EntryAction);

	const TSharedPtr<FMontageGraphSchemaAction_NewNodeSelector> SelectorAction(new FMontageGraphSchemaAction_NewNodeSelector(
		LOCTEXT("MontageGraphNode", "Montage Graph Node"),
		LOCTEXT("AddSelector", "Add Selector..."),
		LOCTEXT("AddSelector",
		        "A new Selector state (suitable to use to branch out HBActions after entry point based on an initial Transition Input)"),
		1
	));
	SelectorAction->NodeTemplate = NewObject<UMontageGraphEdNodeSelector>(ContextMenuBuilder.OwnerOfTemporaries);
	SelectorAction->NodeTemplate->RuntimeNode = NewObject<UMontageGraphNode_Selector>(SelectorAction->NodeTemplate);
	ContextMenuBuilder.AddAction(SelectorAction);

	
	const TSharedPtr<FMontageGraphSchemaAction_NewNodeAction> ActionNodeAction(
		new FMontageGraphSchemaAction_NewNodeAction(
			LOCTEXT("MontageGraphNode", "Montage Graph Node"),
			LOCTEXT("AddAnimation", "AddAnimation..."),
			LOCTEXT("AddAnimation",
			        "AddAnimation"),
			1
		));
	ActionNodeAction->NodeTemplate = NewObject<UMontageGraphEdNodeMontage>(ContextMenuBuilder.OwnerOfTemporaries);
	ActionNodeAction->NodeTemplate->RuntimeNode = NewObject<UMontageGraphNode_Animation>(ActionNodeAction->NodeTemplate);
	ContextMenuBuilder.AddAction(ActionNodeAction);


	UMontageGraph* MontageGraph = CastChecked<UMontageGraph>(ContextMenuBuilder.CurrentGraph->GetOuter());
	// Now, build up actions for anim nodes
	if (!MontageGraph || MontageGraph->NodeType == nullptr)
	{
		return;
	}

	TSet<TSubclassOf<UMontageGraphNode>> Visited;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(MontageGraph->NodeType) && !It->HasAnyClassFlags(CLASS_Abstract) && !Visited.Contains(*It))
		{
			TSubclassOf<UMontageGraphNode> NodeType = *It;
			if (It->GetName().StartsWith("REINST") || It->GetName().StartsWith("SKEL"))
			{
				continue;
			}

			if (NodeType.GetDefaultObject()->ContextMenuName.IsEmpty())
			{
				continue;
			}

			MG_ERROR(Verbose, TEXT("GetGraphContextActions - Create action from %s"), *NodeType->GetName())
			CreateAndAddActionToContextMenu(ContextMenuBuilder, NodeType);
			Visited.Add(NodeType);
		}
	}
}

void UMontageGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetContextMenuActions(Menu, Context);
}

FLinearColor UMontageGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	// light grey
	return FLinearColor(1.f, 0.101961f, 0.101961f, 1.0f);
}

// ReSharper disable once CppConstValueFunctionReturnType
const FPinConnectionResponse UMontageGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
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


	UMontageGraphEdNode* MontageGraphNodeIn = Cast<UMontageGraphEdNode>(OwningNodeA);
	UMontageGraphEdNode* MontageGraphNodeOut = Cast<UMontageGraphEdNode>(OwningNodeB);

	if(!MontageGraphNodeOut->RuntimeNode)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid MontageGraph Node"));
	}


	// Allow wiring from Entry node to Selector Node
	if (OwningNodeA->IsA(UMontageGraphEdNodeEntry::StaticClass()) && OwningNodeB->IsA(UMontageGraphEdNodeSelector::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, LOCTEXT("PinConnectEntryToSelector", "Connect entry node to Selector."));
	}

	// Allow wiring from Selector node to HBAction nodes
	if (OwningNodeA->IsA(UMontageGraphEdNodeSelector::StaticClass()) && MontageGraphNodeOut)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnectEntryToSelector", "Connect Selector node to HBAction node"));
	}

	// Disallow wiring onto an entry node
	if (OwningNodeB->IsA(UMontageGraphEdNodeEntry::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorToEntryNode", "Cannot connect to an entry node (can only create connections **from** entry node)"));
	}

	if (MontageGraphNodeOut == nullptr || MontageGraphNodeIn == nullptr)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid MontageGraph Node"));
	}

	// check for cycles
	FACENodeVisitorCycleChecker CycleChecker;
	if (!CycleChecker.CheckForLoop(OwningNodeA, OwningNodeB))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	FText ErrorMessage;
	if (!MontageGraphNodeIn->RuntimeNode->CanCreateConnectionTo(MontageGraphNodeOut->RuntimeNode, MontageGraphNodeIn->GetOutputPin()->LinkedTo.Num(), ErrorMessage))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}

	if (!MontageGraphNodeOut->RuntimeNode->CanCreateConnectionFrom(MontageGraphNodeIn->RuntimeNode, MontageGraphNodeOut->GetInputPin()->LinkedTo.Num(), ErrorMessage))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes with Transition Input"));

}

bool UMontageGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	// return Super::TryCreateConnection(A, B);

	// We don't actually care about the pin, we want the node that is being dragged between
	UMontageGraphEdNode* NodeA = Cast<UMontageGraphEdNode>(A->GetOwningNode());
	UMontageGraphEdNode* NodeB = Cast<UMontageGraphEdNode>(B->GetOwningNode());

	// Check that this edge doesn't already exist
	for (const UEdGraphPin* TestPin : A->LinkedTo)
	{
		UEdGraphNode* ChildNode = TestPin->GetOwningNode();
		if (const UMontageGraphEdNodeEdge* EdNode_Edge = Cast<UMontageGraphEdNodeEdge>(ChildNode))
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

bool UMontageGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const
{
	UMontageGraphEdNode* NodeA = Cast<UMontageGraphEdNode>(A->GetOwningNode());
	UMontageGraphEdNode* NodeB = Cast<UMontageGraphEdNode>(B->GetOwningNode());

	// Handle conversion from Selector to HBMontageGraph Nodes
	if (const UMontageGraphEdNodeSelector* SelectorNode = Cast<UMontageGraphEdNodeSelector>(A->GetOwningNode()))
	{
		if (NodeB && NodeB->GetInputPin())
		{
			MG_ERROR(Verbose, TEXT("CreateAutomaticConversionNodeAndConnections for Selector"));
			CreateEdgeConnection(A, B, SelectorNode, NodeB);
			return true;
		}
	}

	// Are nodes and pins all valid?
	if (!NodeA || !NodeA->GetOutputPin() || !NodeB || !NodeB->GetInputPin())
	{
		MG_ERROR(Verbose, TEXT("CreateAutomaticConversionNodeAndConnections failed"));
		return false;
	}

	MG_ERROR(Verbose, TEXT("CreateAutomaticConversionNodeAndConnections ok"));
	CreateEdgeConnection(A, B, NodeA, NodeB);

	return true;
}

FConnectionDrawingPolicy* UMontageGraphSchema::CreateConnectionDrawingPolicy(const int32 InBackLayerID, const int32 InFrontLayerID, const float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FMontageGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

void UMontageGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakNodeLinks", "Break Node Links"));
	Super::BreakNodeLinks(TargetNode);

	// UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(&TargetNode);
	// FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

void UMontageGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, const bool bSendsNodeNotification) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));
	Super::BreakPinLinks(TargetPin, bSendsNodeNotification);
	// UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(TargetPin.GetOwningNode());
	// FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

void UMontageGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));
	Super::BreakSinglePinLink(SourcePin, TargetPin);

	// UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(TargetPin->GetOwningNode());
	// FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

UEdGraphPin* UMontageGraphSchema::DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePin, EEdGraphPinDirection InSourcePinDirection) const
{
	const UMontageGraphEdNode* EdGraphNode = Cast<UMontageGraphEdNode>(InTargetNode);
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

bool UMontageGraphSchema::SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const
{
	return Cast<UMontageGraphEdNode>(InTargetNode) != nullptr;
}

bool UMontageGraphSchema::IsCacheVisualizationOutOfDate(const int32 InVisualizationCacheID) const
{
	return CurrentCacheRefreshID != InVisualizationCacheID;
}

int32 UMontageGraphSchema::GetCurrentVisualizationCacheID() const
{
	return CurrentCacheRefreshID;
}

void UMontageGraphSchema::ForceVisualizationCacheClear() const
{
	++CurrentCacheRefreshID;
}

void UMontageGraphSchema::GetGraphDisplayInformation(const UEdGraph& Graph, FGraphDisplayInfo& DisplayInfo) const
{
	Super::GetGraphDisplayInformation(Graph, DisplayInfo);
	DisplayInfo.Tooltip = LOCTEXT("DisplayInfo_Tooltip", "MontageGraph");
}

void UMontageGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const
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

void UMontageGraphSchema::DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const
{
	Super::DroppedAssetsOnNode(Assets, GraphPosition, Node);

	UAnimationAsset* Asset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);
	UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(Node);
	if (Asset && GraphNode && GraphNode->RuntimeNode)
	{
		const FScopedTransaction Transaction(LOCTEXT("UpdateNodeWithAsset", "Updating Node with Asset"));
		GraphNode->Modify();
		// GraphNode->RuntimeNode->SetAnimationAsset(Asset);
		GraphNode->GetSchema()->ForceVisualizationCacheClear();
		GraphNode->ReconstructNode();
	}
}

void UMontageGraphSchema::DroppedAssetsOnPin(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphPin* Pin) const
{
	UAnimationAsset* Asset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);

	if (Asset && Pin)
	{
		FVector2D FixedOffset = GetFixedOffsetFromPin(Pin);

		// if (const UMontageGraphEdNode* EdGraphNode = Cast<UMontageGraphEdNode>(Pin->GetOwningNode()))
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

void UMontageGraphSchema::GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& bOutOkIcon) const
{
	Super::GetAssetsGraphHoverMessage(Assets, HoverGraph, OutTooltipText, bOutOkIcon);

	UAnimationAsset* AnimationAsset = FAssetData::GetFirstAsset<UAnimationAsset>(Assets);
	if (AnimationAsset)
	{
		if (UAnimMontage* Montage = FAssetData::GetFirstAsset<UAnimMontage>(Assets))
		{
			bOutOkIcon = true;
			OutTooltipText = LOCTEXT("MontagesInGraphs", "Drop Montage here to create a new node in the Montage Graph").ToString();
		}
		else if (UAnimSequence* Sequence = FAssetData::GetFirstAsset<UAnimSequence>(Assets))
		{
			bOutOkIcon = true;
			OutTooltipText = LOCTEXT("SequencesInGraphs", "Drop Sequence here to create a new node in the Montage Graph. Montage will be created dynamically from the Anim Sequence.").ToString();
		}
	}
}

void UMontageGraphSchema::GetAssetsPinHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphPin* HoverPin, FString& OutTooltipText, bool& bOutOkIcon) const
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
			OutTooltipText = FText::Format(LOCTEXT("MontagesOnPin", "Connect Montage {0} in the Montage Graph to {1}"),
				FText::FromString(Asset->GetName()),
				OwningNode ? OwningNode->GetNodeTitle(ENodeTitleType::ListView) : FText::FromName(HoverPin->PinName)
			).ToString();
		}
		else if (UAnimSequence* Sequence = Cast<UAnimSequence>(Asset))
		{
			bOutOkIcon = true;
			OutTooltipText = FText::Format(LOCTEXT("SequencesOnPin", "Connect Sequence {0} in the Montage Graph to {1}"),
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

void UMontageGraphSchema::GetAssetsNodeHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphNode* HoverNode, FString& OutTooltipText, bool& bOutOkIcon) const
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

bool UMontageGraphSchema::CanDuplicateGraph(UEdGraph* InSourceGraph) const
{
	return false;
}

int32 UMontageGraphSchema::GetNodeSelectionCount(const UEdGraph* Graph) const
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

TSharedPtr<FEdGraphSchemaAction> UMontageGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FMontageGraphSchemaAction_AddComment));
}

void UMontageGraphSchema::SpawnNodeFromAsset(UAnimationAsset* Asset, const FVector2D& GraphPosition, UEdGraph* Graph, UEdGraphPin* PinIfAvailable)
{
	check(Graph);
	check(Graph->GetSchema()->IsA(UMontageGraphSchema::StaticClass()));
	check(Asset);

	const bool bDirectionMatch = (PinIfAvailable == nullptr) || IsHoverPinMatching(PinIfAvailable);
	if (bDirectionMatch)
	{
		UClass* NewNodeClass = GetNodeClassForAnimAsset(Asset, Graph);
		UClass* NewRuntimeClass = GetRuntimeClassForAnimAsset(Asset, Graph);
		if (NewNodeClass && NewRuntimeClass)
		{
			MG_ERROR(Verbose, TEXT("SpawnNodeFromAsset - NewNodeClass: %s, NewRuntimeClass: %s"), *NewNodeClass->GetName(), *NewRuntimeClass->GetName());
			UMontageGraphEdNode* NewNode = NewObject<UMontageGraphEdNode>(Graph, NewNodeClass);
			NewNode->RuntimeNode = NewObject<UMontageGraphNode>(NewNode, NewRuntimeClass);
			// NewNode->RuntimeNode->SetAnimationAsset(Asset);

			FMontageGraphSchemaAction_NewNode Action;
			Action.NodeTemplate = NewNode;
			Action.PerformAction(Graph, PinIfAvailable, GraphPosition);
		}
	}
}

UClass* UMontageGraphSchema::GetNodeClassForAnimAsset(const UAnimationAsset* Asset, const UEdGraph* Graph)
{
	if (Asset->GetClass()->IsChildOf(UAnimMontage::StaticClass()))
	{
		return UMontageGraphEdNode::StaticClass();
	}

	if (Asset->GetClass()->IsChildOf(UAnimSequence::StaticClass()))
	{
		return UMontageGraphEdNode::StaticClass();
	}

	return nullptr;
}

UClass* UMontageGraphSchema::GetRuntimeClassForAnimAsset(const UAnimationAsset* Asset, const UEdGraph* Graph)
{
	UMontageGraph* MontageGraph = Cast<UMontageGraph>(Graph->GetOuter());
	//
	// if (Asset->GetClass()->IsChildOf(UAnimMontage::StaticClass()))
	// {
	// 	return HBMontageGraph ? HBMontageGraph->DefaultNodeMontageType : UHBMontageGraphNodeMontage::StaticClass();
	// }
	//
	// if (Asset->GetClass()->IsChildOf(UAnimSequence::StaticClass()))
	// {
	// 	return HBMontageGraph ? HBMontageGraph->DefaultNodeSequenceType : UHBMontageGraphNodeSequence::StaticClass();
	// }

	return nullptr;
}

bool UMontageGraphSchema::SupportNodeClassForAsset(UClass* AssetClass, const UEdGraphNode* InGraphNode)
{
	const UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(InGraphNode);
	if (GraphNode && GraphNode->RuntimeNode)
	{
		// See if this node supports this asset type (primary or not)
		// return GraphNode->RuntimeNode->SupportsAssetClass(AssetClass);
	}

	return false;
}

void UMontageGraphSchema::CreateEdgeConnection(UEdGraphPin* PinA, UEdGraphPin* PinB, const UMontageGraphEdNode* OwningNodeA, const UMontageGraphEdNode* OwningNodeB) const
{
	const FVector2D InitPos((OwningNodeA->NodePosX + OwningNodeB->NodePosX) / 2, (OwningNodeA->NodePosY + OwningNodeB->NodePosY) / 2);

	// UInputAction* DefaultInput = nullptr;
	// if (const UMontageGraphEdGraph* HBMontageGraph = Cast<UMontageGraphEdGraph>(OwningNodeA->GetGraph()))
	// {
	// 	if (const UHBMontageGraph* HBMontageGraphModel = HBMontageGraph->GetMontageGraphModel())
	// 	{
	// 		DefaultInput = HBMontageGraphModel->DefaultInputAction;
	// 	}
	// }

	FMontageGraphSchemaAction_NewEdge Action;
	Action.NodeTemplate = NewObject<UMontageGraphEdNodeEdge>(OwningNodeA->GetGraph());

	UMontageGraphEdge* RuntimeEdge = NewObject<UMontageGraphEdge>(Action.NodeTemplate, UMontageGraphEdge::StaticClass());
	// RuntimeEdge->TransitionInput = DefaultInput;
	Action.NodeTemplate->SetRuntimeEdge(RuntimeEdge);

	Action.PerformAction(OwningNodeA->GetGraph(), PinB, InitPos, true);
	// Always create connections from node A to B, don't allow adding in reverse
	Action.NodeTemplate->CreateConnections(OwningNodeA, OwningNodeB);
	
}

void UMontageGraphSchema::CreateAndAddActionToContextMenu(FGraphContextMenuBuilder& ContextMenuBuilder, const TSubclassOf<UMontageGraphNode> NodeType) const
{
	if (!NodeType)
	{
		return;
	}


	UMontageGraphNode* NodeCDO = NodeType.GetDefaultObject();
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
	FText Category = NodeType->IsNative() ? LOCTEXT("CombatGraphNodeAction", "Montage Graph Node") : FText::FromString(TEXT("Custom"));

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

	const TSharedPtr<FMontageGraphSchemaAction_NewNode> Action(new FMontageGraphSchemaAction_NewNode(Category, Description, AddToolTip, 1));
	Action->NodeTemplate = NewObject<UMontageGraphEdNode>(ContextMenuBuilder.OwnerOfTemporaries, "Template");
	Action->NodeTemplate->RuntimeNode = NewObject<UMontageGraphNode>(Action->NodeTemplate, NodeType);
	ContextMenuBuilder.AddAction(Action);
}

bool UMontageGraphSchema::IsHoverPinMatching(const UEdGraphPin* InHoverPin)
{
	check(InHoverPin);

	const bool bIsValidName = InHoverPin->PinName == UMontageGraphPinNames::PinName_In ||
		InHoverPin->PinName == UMontageGraphPinNames::PinName_Out;

	return (InHoverPin->Direction == EGPD_Input || InHoverPin->Direction == EGPD_Output) && bIsValidName;
}

FVector2D UMontageGraphSchema::GetFixedOffsetFromPin(const UEdGraphPin* InHoverPin)
{
	// If don't have access to bounding information for node, using fixed offset that should work for most cases.
	// Start at 450.f, and is further refined based on SlateNode bounding info is available
	FVector2D FixedOffset(450.0f, 0.f);
	FVector2D DesiredSize(0.f, 0.f);

	if (const UMontageGraphEdNode* EdGraphNode = Cast<UMontageGraphEdNode>(InHoverPin->GetOwningNode()))
	{
		if (EdGraphNode->SlateNode)
		{
			DesiredSize = EdGraphNode->SlateNode->ComputeDesiredSize(FSlateApplication::Get().GetApplicationScale());
		}
	}

	// if (InHoverPin->PinName == UMontageGraphPinNames::PinName_In)
	// {
	// 	FixedOffset = FVector2D(-(DesiredSize.X * 2 + 100.f), -DesiredSize.Y);
	// }
	// else if (InHoverPin->PinName == UMontageGraphPinNames::PinName_In)
	// {
	// 	FixedOffset = FVector2D(-DesiredSize.X, -(DesiredSize.Y * 2 + 100.f));
	// }
	// else if (InHoverPin->PinName == UMontageGraphPinNames::PinName_Out)
	// {
	// 	FixedOffset = FVector2D(100.f, -DesiredSize.Y);
	// }
	// else if (InHoverPin->PinName == UMontageGraphPinNames::PinName_Out)
	// {
	// 	FixedOffset = FVector2D(-DesiredSize.X, 100.f);
	// }


	return DesiredSize + FixedOffset;
}

#undef LOCTEXT_NAMESPACE
