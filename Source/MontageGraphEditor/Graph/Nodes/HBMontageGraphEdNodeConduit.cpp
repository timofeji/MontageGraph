#include "HBMontageGraphEdNodeConduit.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/MontageGraphNode_Conduit.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphEdNodeConduit"

UMontageGraphEdNodeConduit::UMontageGraphEdNodeConduit()
{
	bCanRenameNode = false;
}

void UMontageGraphEdNodeConduit::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	if (UMontageGraphNode_Conduit* ConduitNode{Cast<UMontageGraphNode_Conduit>(RuntimeNode)})
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
			CreatePin(EGPD_Output, UMontageGraphPinNames::PinName_Out, FName("None"));
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

void UMontageGraphEdNodeConduit::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMontageGraphPinNames::PinName_In, TEXT("In"));
}

void UMontageGraphEdNodeConduit::AutowireNewNode(UEdGraphPin* FromPin)
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

FText UMontageGraphEdNodeConduit::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("ConduitNodeTitle", "Conduit");
}

FText UMontageGraphEdNodeConduit::GetTooltipText() const
{
	return LOCTEXT("ConduitNodeTooltip",
	               "This is a conduit, which allows branching out beginning of Montage Graph based on an initial Transition Input");
}

void UMontageGraphEdNodeConduit::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UMontageGraphEdNodeConduit::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UMontageGraphEdNodeConduit::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

void UMontageGraphEdNodeConduit::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
}

#undef LOCTEXT_NAMESPACE
