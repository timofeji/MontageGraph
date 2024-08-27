
#include "HBActionGraphDragDropAction.h"

#include "EditorStyleSet.h"
#include "SGraphPanel.h"
#include "SGraphPin.h"
#include "Nodes/ActionGraphEdNode.h"

#define LOCTEXT_NAMESPACE "ACEGraphDragAction"

FHBActionGraphDragDropAction::FHBActionGraphDragDropAction(const TSharedRef<SGraphPanel>& GraphPanel, TArray<FGraphPinHandle>& DraggedPins)
	: GraphPanel(GraphPanel),
	  DraggingPins(DraggedPins),
	  DecoratorAdjust(FSlateApplication::Get().GetCursorSize())
{
	if (DraggingPins.Num() > 0)
	{
		for (FGraphPinHandle DraggedPin : DraggedPins)
		{
			UEdGraphPin* Pin = DraggedPin.GetPinObj(*GraphPanel);
			if (Pin && Pin->Direction == EGPD_Input)
			{
				DecoratorAdjust = DecoratorAdjust * FVector2D(-1.f, 1.f);
			}
		}
	}

	for (const FGraphPinHandle GraphPinHandle : DraggedPins)
	{
		GraphPanel->OnBeginMakingConnection(GraphPinHandle);
	}
}

TSharedRef<FHBActionGraphDragDropAction> FHBActionGraphDragDropAction::New(const TSharedRef<SGraphPanel>& GraphPanel, TArray<FGraphPinHandle>& InStartingPins)
{
	TSharedRef<FHBActionGraphDragDropAction> Operation = MakeShareable(new FHBActionGraphDragDropAction(GraphPanel, InStartingPins));
	Operation->Construct();
	return Operation;
}

void FHBActionGraphDragDropAction::OnDrop(const bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	GraphPanel->OnStopMakingConnection();
	FGraphEditorDragDropAction::OnDrop(bDropWasHandled, MouseEvent);
}

void FHBActionGraphDragDropAction::HoverTargetChanged()
{
	TArray<FPinConnectionResponse> Responses;

	if (const UEdGraphPin* TargetPinObj = GetHoveredPin())
	{
		AppendConnectionResponsesForHoveredPin(TargetPinObj, Responses);
	}
	else if (const UActionGraphEdNode* TargetNode = Cast<UActionGraphEdNode>(GetHoveredNode()))
	{
		AppendConnectionResponsesForHoveredNode(TargetNode, Responses);
	}
	else if (const UEdGraph* TargetGraph = GetHoveredGraph())
	{
		AppendConnectionResponsesForHoveredGraph(TargetGraph, Responses);
	}

	// Let the user know the status of dropping now
	if (Responses.Num() == 0)
	{
		// Display the place a new node icon, we're not over a valid pin and have no message from the schema
		SetSimpleFeedbackMessage(
			FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.NewNode")),
			FLinearColor::White,
			NSLOCTEXT("GraphEditor.Feedback", "PlaceNewNode", "Place a new node.")
		);

		return;
	}

	// Take the unique responses and create visual feedback for it
	const TSharedRef<SVerticalBox> FeedbackBox = SNew(SVerticalBox);
	for (FPinConnectionResponse Response : Responses)
	{
		const FSlateBrush* StatusSymbol = nullptr;

		switch (Response.Response)
		{
		case CONNECT_RESPONSE_MAKE:
		case CONNECT_RESPONSE_BREAK_OTHERS_A:
		case CONNECT_RESPONSE_BREAK_OTHERS_B:
		case CONNECT_RESPONSE_BREAK_OTHERS_AB:
			StatusSymbol = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK"));
			break;

		case CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE:
			StatusSymbol = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.ViaCast"));
			break;

		case CONNECT_RESPONSE_DISALLOW:
		default:
			StatusSymbol = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
			break;
		}

		// Add a new message row
		FeedbackBox->AddSlot()
	   .AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .Padding(3.0f)
			  .VAlign(VAlign_Center)
			[
				SNew(SImage).Image(StatusSymbol)
			]
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Response.Message)
			]
		];
	}

	SetFeedbackMessage(FeedbackBox);
}

void FHBActionGraphDragDropAction::OnDragged(const FDragDropEvent& DragDropEvent)
{
	const FVector2D TargetPosition = DragDropEvent.GetScreenSpacePosition();

	// Reposition the info window wrt to the drag
	CursorDecoratorWindow->MoveWindowTo(DragDropEvent.GetScreenSpacePosition() + DecoratorAdjust);
	// Request the active panel to scroll if required
	GraphPanel->RequestDeferredPan(TargetPosition);
}

FReply FHBActionGraphDragDropAction::DroppedOnPin(FVector2D ScreenPosition, FVector2D GraphPosition)
{
	TArray<UEdGraphPin*> ValidSourcePins;
	ValidateGraphPinList(ValidSourcePins);

	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_CreateConnection", "Create Pin Link"));

	UEdGraphPin* PinB = GetHoveredPin();
	bool bError = false;
	TSet<UEdGraphNode*> NodeList;

	for (UEdGraphPin* PinA : ValidSourcePins)
	{
		if ((PinA != nullptr) && (PinB != nullptr))
		{
			const UEdGraph* MyGraphObj = PinA->GetOwningNode()->GetGraph();

			if (PinB->Direction == EGPD_Output)
			{
				MyGraphObj->GetSchema()->TryCreateConnection(PinA, PinB->GetOwningNode()->GetPinAt(0));
			}
			else
			{
				if (MyGraphObj->GetSchema()->TryCreateConnection(PinA, PinB))
				{
					if (!PinA->IsPendingKill())
					{
						NodeList.Add(PinA->GetOwningNode());
					}
					if (!PinB->IsPendingKill())
					{
						NodeList.Add(PinB->GetOwningNode());
					}
				}
			}
		}
		else
		{
			bError = true;
		}
	}

	// Send all nodes that received a new pin connection a notification
	for (auto It = NodeList.CreateConstIterator(); It; ++It)
	{
		UEdGraphNode* Node = (*It);
		Node->NodeConnectionListChanged();
	}

	if (bError)
	{
		return FReply::Unhandled();
	}

	return FReply::Handled();
}

FReply FHBActionGraphDragDropAction::DroppedOnNode(FVector2D ScreenPosition, FVector2D GraphPosition)
{
	bool bHandledPinDropOnNode = false;
	UEdGraphNode* NodeOver = GetHoveredNode();

	if (NodeOver)
	{
		// Gather any source drag pins
		TArray<UEdGraphPin*> ValidSourcePins;
		ValidateGraphPinList(ValidSourcePins);

		if (ValidSourcePins.Num())
		{
			for (UEdGraphPin* SourcePin : ValidSourcePins)
			{
				// Check for pin drop support
				FText ResponseText;
				if (SourcePin->GetOwningNode() != NodeOver && SourcePin->GetSchema()->SupportsDropPinOnNode(NodeOver, SourcePin->PinType, SourcePin->Direction, ResponseText))
				{
					bHandledPinDropOnNode = true;

					// Find which pin name to use and drop the pin on the node
					const FName PinName = SourcePin->PinFriendlyName.IsEmpty() ? SourcePin->PinName : *SourcePin->PinFriendlyName.ToString();

					const FScopedTransaction Transaction((SourcePin->Direction == EGPD_Output) ? NSLOCTEXT("UnrealEd", "AddInParam", "Add In Parameter") : NSLOCTEXT("UnrealEd", "AddOutParam", "Add Out Parameter"));

					UEdGraphPin* TargetPin = NodeOver->GetSchema()->DropPinOnNode(GetHoveredNode(), PinName, SourcePin->PinType, SourcePin->Direction);

					// This can invalidate the source pin due to node reconstruction, abort in that case
					if (SourcePin->GetOwningNodeUnchecked() && TargetPin)
					{
						SourcePin->Modify();
						TargetPin->Modify();
						SourcePin->GetSchema()->TryCreateConnection(SourcePin, TargetPin);
					}
				}

				// If we have not handled the pin drop on node and there is an error message, do not let other actions occur.
				if (!bHandledPinDropOnNode && !ResponseText.IsEmpty())
				{
					bHandledPinDropOnNode = true;
				}
			}
		}
	}
	return bHandledPinDropOnNode ? FReply::Handled() : FReply::Unhandled();
}

FReply FHBActionGraphDragDropAction::DroppedOnPanel(const TSharedRef<SWidget>& Panel, const FVector2D ScreenPosition, const FVector2D GraphPosition, UEdGraph& Graph)
{
	// Gather any source drag pins
	TArray<UEdGraphPin*> PinObjects;
	ValidateGraphPinList(PinObjects);

	// Create a context menu
	const TSharedPtr<SWidget> WidgetToFocus = GraphPanel->SummonContextMenu(ScreenPosition, GraphPosition, nullptr, nullptr, PinObjects);

	// Give the context menu focus
	return (WidgetToFocus.IsValid())
		? FReply::Handled().SetUserFocus(WidgetToFocus.ToSharedRef(), EFocusCause::SetDirectly)
		: FReply::Handled();
}

void FHBActionGraphDragDropAction::ValidateGraphPinList(TArray<UEdGraphPin*>& OutValidPins)
{
	OutValidPins.Empty(DraggingPins.Num());

	for (FGraphPinHandle DraggingPin : DraggingPins)
	{
		if (UEdGraphPin* Pin = DraggingPin.GetPinObj(*GraphPanel))
		{
			OutValidPins.Add(Pin);
		}
	}
}

void FHBActionGraphDragDropAction::AppendConnectionResponsesForHoveredPin(const UEdGraphPin* TargetPin, TArray<FPinConnectionResponse>& OutResponses)
{
	TArray<UEdGraphPin*> ValidSourcePins;
	ValidateGraphPinList(ValidSourcePins);

	// Check the schema for connection responses
	for (const UEdGraphPin* StartingPinObj : ValidSourcePins)
	{
		// The Graph object in which the pins reside.
		const UEdGraph* GraphObj = StartingPinObj->GetOwningNode()->GetGraph();

		// Determine what the schema thinks about the wiring action
		const FPinConnectionResponse Response = GraphObj->GetSchema()->CanCreateConnection(StartingPinObj, TargetPin);

		if (Response.Response == CONNECT_RESPONSE_DISALLOW)
		{
			TSharedPtr<SGraphNode> NodeWidget = TargetPin->GetOwningNode()->DEPRECATED_NodeWidget.Pin();
			if (NodeWidget.IsValid())
			{
				NodeWidget->NotifyDisallowedPinConnection(StartingPinObj, TargetPin);
			}
		}

		OutResponses.AddUnique(Response);
	}
}

void FHBActionGraphDragDropAction::AppendConnectionResponsesForHoveredNode(const UActionGraphEdNode* TargetNode, TArray<FPinConnectionResponse>& OutResponses)
{
	TArray<UEdGraphPin*> StartingPins;
	ValidateGraphPinList(StartingPins);

	FPinConnectionResponse Response;

	for (const UEdGraphPin* StartingPin : StartingPins)
	{
		const UEdGraphSchema* Schema = StartingPin->GetSchema();
		UEdGraphPin* TargetInputPin = TargetNode->GetInputPin();

		if (Schema && TargetInputPin)
		{
			Response = Schema->CanCreateConnection(StartingPin, TargetInputPin);
			if (Response.Response == CONNECT_RESPONSE_DISALLOW)
			{
				TSharedPtr<SGraphNode> NodeWidget = TargetInputPin->GetOwningNode()->DEPRECATED_NodeWidget.Pin();
				if (NodeWidget.IsValid())
				{
					NodeWidget->NotifyDisallowedPinConnection(StartingPin, TargetInputPin);
				}
			}
		}
		else
		{
			Response = FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid Action Graph Node"));
		}

		OutResponses.Add(Response);
	}
}

void FHBActionGraphDragDropAction::AppendConnectionResponsesForHoveredGraph(const UEdGraph* HoverGraph, TArray<FPinConnectionResponse>& OutResponses)
{
	TArray<UEdGraphPin*> ValidSourcePins;
	ValidateGraphPinList(ValidSourcePins);

	for (UEdGraphPin* StartingPinObj : ValidSourcePins)
	{
		// Let the schema describe the connection we might make
		FPinConnectionResponse Response = HoverGraph->GetSchema()->CanCreateNewNodes(StartingPinObj);
		if (!Response.Message.IsEmpty())
		{
			OutResponses.AddUnique(Response);
		}
	}
}

#undef LOCTEXT_NAMESPACE
