#include "MontageGraphEdNodeSelector.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/MontageGraphNode_Selector.h"

#define LOCTEXT_NAMESPACE "MontageGraphEdNodeSelector"

UMontageGraphEdNodeSelector::UMontageGraphEdNodeSelector()
{
	bCanRenameNode = false;
}

void UMontageGraphEdNodeSelector::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	if (UMontageGraphNode_Selector* SelectorNode{Cast<UMontageGraphNode_Selector>(RuntimeNode)})
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

		int MaxSelectorIndex = SelectorNode->ConductStates.Num();
		if (MaxSelectorIndex > OutputPins.Num())
		{
			CreatePin(EGPD_Output, UMontageGraphPinNames::PinName_Out, FName("None"));
		}
		else if (MaxSelectorIndex < OutputPins.Num())
		{
			for (int i = OutputPins.Num(); i > MaxSelectorIndex; i--)
			{
				RemovePinAt(i - 1, EGPD_Output);
			}
		}


		//Rename pins
		for (int i = 0; i < MaxSelectorIndex; i++)
		{
			UEdGraphPin* Pin = GetPinAt(i + 1);
			if (Pin)
			{
				Pin->PinName = SelectorNode->ConductStates[i].GetTagName();
			}
		}

		GetGraph()->NotifyGraphChanged();
	}
}

void UMontageGraphEdNodeSelector::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMontageGraphPinNames::PinName_In, TEXT("In"));
}

void UMontageGraphEdNodeSelector::AutowireNewNode(UEdGraphPin* FromPin)
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

FText UMontageGraphEdNodeSelector::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("SelectorNodeTitle", "Selector");
}

FText UMontageGraphEdNodeSelector::GetTooltipText() const
{
	return LOCTEXT("SelectorNodeTooltip",
	               "This is a Selector, which allows branching out beginning of Montage Graph based on an initial Transition Input");
}

void UMontageGraphEdNodeSelector::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UMontageGraphEdNodeSelector::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UMontageGraphEdNodeSelector::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

void UMontageGraphEdNodeSelector::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
}

#undef LOCTEXT_NAMESPACE
