#include "HBActionGraphEdNodeMontage.h"
#include "ActionGraphEditorTypes.h"

#define LOCTEXT_NAMESPACE "HBActionGraphEdNodeMontage"

UActionGraphEdNodeMontage::UActionGraphEdNodeMontage()
{
	bCanRenameNode = false;
}

void UActionGraphEdNodeMontage::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UActionGraphPinNames::PinName_In, TEXT("In"));
	CreatePin(EGPD_Output, UActionGraphPinNames::PinName_Out, TEXT("Out"));
}

void UActionGraphEdNodeMontage::AutowireNewNode(UEdGraphPin* FromPin)
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

FText UActionGraphEdNodeMontage::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Action Montage", "Action");
}

FText UActionGraphEdNodeMontage::GetTooltipText() const
{
	return LOCTEXT("Action Montage Tooltip", "This is a conduit, which allows branching out beginning of HBAction graph based on an initial Transition Input");
}

void UActionGraphEdNodeMontage::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UActionGraphEdNodeMontage::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UActionGraphEdNodeMontage::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

#undef LOCTEXT_NAMESPACE
