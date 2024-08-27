// Created by Timofej Jermolaev, All rights reserved . 


#include "HBActionGraphEdNodeSwitch.h"
#include "ActionGraphEditorTypes.h"

#define LOCTEXT_NAMESPACE "HBActionGraphEdNodeSwitch"

UActionGraphEdNodeSwitch::UActionGraphEdNodeSwitch()
{
	bCanRenameNode = false;
}

void UActionGraphEdNodeSwitch::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UActionGraphPinNames::PinCategory_Transition, TEXT("In"));
	CreatePin(EGPD_Output, UActionGraphPinNames::PinCategory_Transition, TEXT("Out"));
}

void UActionGraphEdNodeSwitch::AutowireNewNode(UEdGraphPin* FromPin)
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

FText UActionGraphEdNodeSwitch::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Switch", "Switch");
}

FText UActionGraphEdNodeSwitch::GetTooltipText() const
{
	return LOCTEXT("Switch", "This is a conduit, which allows branching out beginning of HBAction graph based on an initial Transition Input");
}

void UActionGraphEdNodeSwitch::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UActionGraphEdNodeSwitch::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UActionGraphEdNodeSwitch::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

#undef LOCTEXT_NAMESPACE
