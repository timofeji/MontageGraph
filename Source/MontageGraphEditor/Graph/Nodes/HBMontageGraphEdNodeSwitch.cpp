// Created by Timofej Jermolaev, All rights reserved . 


#include "HBMontageGraphEdNodeSwitch.h"
#include "MontageGraphEditorTypes.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphEdNodeSwitch"

UMontageGraphEdNodeSwitch::UMontageGraphEdNodeSwitch()
{
	bCanRenameNode = false;
}

void UMontageGraphEdNodeSwitch::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMontageGraphPinNames::PinCategory_Transition, TEXT("In"));
	CreatePin(EGPD_Output, UMontageGraphPinNames::PinCategory_Transition, TEXT("Out"));
}

void UMontageGraphEdNodeSwitch::AutowireNewNode(UEdGraphPin* FromPin)
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

FText UMontageGraphEdNodeSwitch::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Switch", "Switch");
}

FText UMontageGraphEdNodeSwitch::GetTooltipText() const
{
	return LOCTEXT("Switch", "This is a Selector, which allows branching out beginning of Montage Graph based on an initial Transition Input");
}

void UMontageGraphEdNodeSwitch::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UMontageGraphEdNodeSwitch::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UMontageGraphEdNodeSwitch::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

#undef LOCTEXT_NAMESPACE
