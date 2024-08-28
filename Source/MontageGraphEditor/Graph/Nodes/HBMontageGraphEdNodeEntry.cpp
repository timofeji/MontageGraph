#include "HBMontageGraphEdNodeEntry.h"

#include "MontageGraphEditorTypes.h"
#include "MontageGraph/MontageGraphNode_Entry.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphEdNodeEntry"

UMontageGraphEdNodeEntry::UMontageGraphEdNodeEntry()
{
	bCanRenameNode = false;
}

void UMontageGraphEdNodeEntry::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (auto EntryNode{Cast<UMontageGraphNode_Entry>(RuntimeNode)})
	{
		Pins[0]->PinName = EntryNode->EntryTag.GetTagName();
	}
}

void UMontageGraphEdNodeEntry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UMontageGraphPinNames::PinName_Out, TEXT("Action Entry"));
}

FText UMontageGraphEdNodeEntry::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UEdGraph* Graph = GetGraph();
	return FText::FromString(Graph->GetName());
}

FText UMontageGraphEdNodeEntry::GetTooltipText() const
{
	return LOCTEXT("StateEntryNodeTooltip", "Entry point for state machine");
}

UEdGraphNode* UMontageGraphEdNodeEntry::GetOutputNode()
{
	if (Pins.Num() > 0 && Pins[0] != nullptr)
	{
		check(Pins[0]->LinkedTo.Num() <= 1);
		if (Pins[0]->LinkedTo.Num() > 0 && Pins[0]->LinkedTo[0]->GetOwningNode() != nullptr)
		{
			return Pins[0]->LinkedTo[0]->GetOwningNode();
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
