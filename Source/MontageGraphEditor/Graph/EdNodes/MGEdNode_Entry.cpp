#include "MGEdNode_Entry.h"

#include "MontageGraphEditorStyle.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/Nodes/MontageGraphNode_Entry.h"

#define LOCTEXT_NAMESPACE "MGEdNode_Entry"

UMGEdNode_Entry::UMGEdNode_Entry()
{
	bCanRenameNode = false;
}

FLinearColor UMGEdNode_Entry::GetNodeTitleColor() const
{
	return FLinearColor(FColor::FromHex(TEXT("#FAF8FFFF")));
}


FSlateIcon UMGEdNode_Entry::GetIconAndTint(FLinearColor& OutColor) const
{
	// return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "Graph.Node.Icon.Bind");
	 return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Node.Icon.Selector");
}


void UMGEdNode_Entry::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (auto EntryNode{Cast<UMontageGraphNode_Entry>(RuntimeNode)})
	{
		Pins[0]->PinName = EntryNode->EntryTag.GetTagName();
	}
}

void UMGEdNode_Entry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UMontageGraphPinNames::PinName_Out, TEXT("Entry"));
}

FText UMGEdNode_Entry::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (auto EntryNode{Cast<UMontageGraphNode_Entry>(RuntimeNode)})
	{
		return FText::FromString(EntryNode->EntryTag.ToString());
	}
	
	return FText::FromString("Entry Node");
}

FText UMGEdNode_Entry::GetTooltipText() const
{
	return LOCTEXT("StateEntryNodeTooltip", "Entry point for state machine");
}

UEdGraphNode* UMGEdNode_Entry::GetOutputNode()
{
	if (Pins.Num() > 0 && Pins[0] != nullptr)
	{
		if (Pins[0]->LinkedTo.Num() > 0)
		{
			return Pins[0]->LinkedTo[0]->GetOwningNode();
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
