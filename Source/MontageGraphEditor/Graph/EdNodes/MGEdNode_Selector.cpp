#include "MGEdNode_Selector.h"

#include "MontageGraphEditorStyle.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/Nodes/MGNode_Selector.h"

#define LOCTEXT_NAMESPACE "MGEdNode_Selector"

UMGEdNode_Selector::UMGEdNode_Selector()
{
	bCanRenameNode = false;
}

FLinearColor UMGEdNode_Selector::GetNodeTitleColor() const
{
	return FLinearColor(FColor::FromHex(TEXT("#252525FF")));
}

FSlateIcon UMGEdNode_Selector::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Node.Icon.Selector");
}

//
// void UMGEdNode_Selector::PostEditChangeProperty(
// 	FPropertyChangedEvent& PropertyChangedEvent)
// {
// 	Super::PostEditChangeProperty(PropertyChangedEvent);
//
// 	if (!PropertyChangedEvent.Property)
// 	{
// 		return;
// 	}
//
// 	if (UMGNode_Selector* SelectorNode{Cast<UMGNode_Selector>(RuntimeNode)})
// 	{
// 		//Delete or Add extra pins
// 		TArray<UEdGraphPin*> OutputPins;
// 		for (UEdGraphPin* Pin : Pins)
// 		{
// 			if (Pin->Direction == EGPD_Output)
// 			{
// 				OutputPins.Add(Pin);
// 			}
// 		}
//
// 		int MaxSelectorIndex = SelectorNode->SelectorStates.Num();
// 		if (MaxSelectorIndex > OutputPins.Num())
// 		{
// 			CreatePin(EGPD_Output, UMontageGraphPinNames::PinName_Out, FName("None"));
// 		}
// 		else if (MaxSelectorIndex < OutputPins.Num())
// 		{
// 			for (int i = OutputPins.Num(); i > MaxSelectorIndex; i--)
// 			{
// 				RemovePinAt(i - 1, EGPD_Output);
// 			}
// 		}
//
//
// 		//Rename pins
// 		for (int i = 0; i < MaxSelectorIndex; i++)
// 		{
// 			UEdGraphPin* Pin = GetPinAt(i + 1);
// 			if (Pin)
// 			{
// 				Pin->PinName = SelectorNode->SelectorStates[i].GetTagName();
// 			}
// 		}
//
// 		GetGraph()->NotifyGraphChanged();
// 	}
// }
//
// void UMGEdNode_Selector::AllocateDefaultPins()
// {
// 	CreatePin(EGPD_Input, UMontageGraphPinNames::PinName_In, TEXT("In"));
// }
//
// void UMGEdNode_Selector::AutowireNewNode(UEdGraphPin* FromPin)
// {
// 	Super::AutowireNewNode(FromPin);
//
// 	if (FromPin)
// 	{
// 		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
// 		{
// 			FromPin->GetOwningNode()->NodeConnectionListChanged();
// 		}
// 	}
// }
//
FText UMGEdNode_Selector::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("SelectorNodeTitle", "Selector");
}

FText UMGEdNode_Selector::GetTooltipText() const
{
	return LOCTEXT("SelectorNodeTooltip",
	               "This is a Selector, which allows branching out beginning of Montage Graph based on an initial Transition Input");
}

void UMGEdNode_Selector::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UMGEdNode_Selector::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UMGEdNode_Selector::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

void UMGEdNode_Selector::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();
}

#undef LOCTEXT_NAMESPACE
