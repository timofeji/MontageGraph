#include "HBActionGraphEdNodeEntry.h"

#include "ActionGraphEditorTypes.h"
#include "ActionGraph/ActionGraphNode_Entry.h"

#define LOCTEXT_NAMESPACE "HBActionGraphEdNodeEntry"

UActionGraphEdNodeEntry::UActionGraphEdNodeEntry()
{
	bCanRenameNode = false;
}

void UActionGraphEdNodeEntry::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (auto EntryNode{Cast<UActionGraphNode_Entry>(RuntimeNode)})
	{
		Pins[0]->PinName = EntryNode->EntryTag.GetTagName();
	}
}

void UActionGraphEdNodeEntry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UActionGraphPinNames::PinName_Out, TEXT("Action Entry"));
}

FText UActionGraphEdNodeEntry::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UEdGraph* Graph = GetGraph();
	return FText::FromString(Graph->GetName());
}

FText UActionGraphEdNodeEntry::GetTooltipText() const
{
	return LOCTEXT("StateEntryNodeTooltip", "Entry point for state machine");
}

UEdGraphNode* UActionGraphEdNodeEntry::GetOutputNode()
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
