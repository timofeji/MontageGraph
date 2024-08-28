#include "HBMontageGraphEdNodeMontage.h"
#include "MontageGraphEditorTypes.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphEdNodeMontage"

UMontageGraphEdNodeMontage::UMontageGraphEdNodeMontage()
{
	bCanRenameNode = false;
}

void UMontageGraphEdNodeMontage::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMontageGraphPinNames::PinName_In, TEXT("In"));
	CreatePin(EGPD_Output, UMontageGraphPinNames::PinName_Out, TEXT("Out"));
}

void UMontageGraphEdNodeMontage::AutowireNewNode(UEdGraphPin* FromPin)
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

FText UMontageGraphEdNodeMontage::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Action Montage", "Action");
}

FText UMontageGraphEdNodeMontage::GetTooltipText() const
{
	return LOCTEXT("Action Montage Tooltip", "This is a conduit, which allows branching out beginning of Montage Graph based on an initial Transition Input");
}

void UMontageGraphEdNodeMontage::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UMontageGraphEdNodeMontage::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UMontageGraphEdNodeMontage::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

#undef LOCTEXT_NAMESPACE
