#include "HBActionGraphEdNodeConduit.h"
#include "ActionGraphEditorTypes.h"
#include "ActionGraph/ActionGraphNode_Conduit.h"

#define LOCTEXT_NAMESPACE "HBActionGraphEdNodeConduit"

UActionGraphEdNodeConduit::UActionGraphEdNodeConduit()
{
	bCanRenameNode = false;
}

void UActionGraphEdNodeConduit::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	if (UActionGraphNode_Conduit* ConduitNode{Cast<UActionGraphNode_Conduit>(RuntimeNode)})
	{
		//Delete or Add extra pins
		TArray<UEdGraphPin*> OutputPins;
		for (UEdGraphPin* Pin : Pins)
		{
			if (Pin->Direction == EGPD_Output)
			{
				OutputPins.Add(Pin);
			}
		}

		int MaxConduitIndex = ConduitNode->ConductStates.Num();
		if (MaxConduitIndex > OutputPins.Num())
		{
			CreatePin(EGPD_Output, UActionGraphPinNames::PinName_Out, FName("None"));
		}
		else if (MaxConduitIndex < OutputPins.Num())
		{
			for (int i = OutputPins.Num(); i > MaxConduitIndex; i--)
			{
				RemovePinAt(i - 1, EGPD_Output);
			}
		}


		//Rename pins
		for (int i = 0; i < MaxConduitIndex; i++)
		{
			UEdGraphPin* Pin = GetPinAt(i + 1);
			if (Pin)
			{
				Pin->PinName = ConduitNode->ConductStates[i].GetTagName();
			}
		}

		GetGraph()->NotifyGraphChanged();
	}
}

void UActionGraphEdNodeConduit::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UActionGraphPinNames::PinName_In, TEXT("In"));
}

void UActionGraphEdNodeConduit::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin)
	{
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

FText UActionGraphEdNodeConduit::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("ConduitNodeTitle", "Conduit");
}

FText UActionGraphEdNodeConduit::GetTooltipText() const
{
	return LOCTEXT("ConduitNodeTooltip",
	               "This is a conduit, which allows branching out beginning of HBAction graph based on an initial Transition Input");
}

void UActionGraphEdNodeConduit::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UActionGraphEdNodeConduit::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UActionGraphEdNodeConduit::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

void UActionGraphEdNodeConduit::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
}

#undef LOCTEXT_NAMESPACE
