

#include "HBMontageGraphEdNodeEdge.h"

#include "MontageGraphEditorTypes.h"

#include "MontageGraph/MontageGraphEdge.h"
#include "MontageGraphEdNode.h"
#include "HBMontageGraphEdNodeSelector.h"
#include "..\..\MontageGraphEditorLog.h"

UMontageGraphEdNodeEdge::UMontageGraphEdNodeEdge()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UMontageGraphEdNodeEdge::OnObjectPropertyChanged);
}

UMontageGraphEdNodeEdge::~UMontageGraphEdNodeEdge()
{
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}

void UMontageGraphEdNodeEdge::SetRuntimeEdge(UMontageGraphEdge* InEdge)
{
	RuntimeEdge = InEdge;
	UpdateCachedIcon();
}

void UMontageGraphEdNodeEdge::CreateConnections(const UMontageGraphEdNode* Start, const UMontageGraphEdNode* End)
{
	CreateBaseConnections(Start, End);
}

void UMontageGraphEdNodeEdge::CreateBaseConnections(const UMontageGraphEdNode* Start, const UMontageGraphEdNode* End)
{
	Pins[0]->Modify();
	Pins[0]->LinkedTo.Empty();

	Start->GetOutputPin()->Modify();
	Pins[0]->MakeLinkTo(Start->GetOutputPin());

	// This to next
	Pins[1]->Modify();
	Pins[1]->LinkedTo.Empty();

	End->GetInputPin()->Modify();
	Pins[1]->MakeLinkTo(End->GetInputPin());
}

UMontageGraphEdNode* UMontageGraphEdNodeEdge::GetStartNodeAsBase() const
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UMontageGraphEdNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

UMontageGraphEdNode* UMontageGraphEdNodeEdge::GetStartNodeAsGraphNode() const
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UMontageGraphEdNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

UMontageGraphEdNodeSelector* UMontageGraphEdNodeEdge::GetStartNodeAsSelector() const
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UMontageGraphEdNodeSelector>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

UMontageGraphEdNode* UMontageGraphEdNodeEdge::GetEndNode() const
{
	if (Pins[1]->LinkedTo.Num() > 0)
	{
		return Cast<UMontageGraphEdNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

void UMontageGraphEdNodeEdge::AllocateDefaultPins()
{
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, UMontageGraphPinNames::PinCategory_Edge, FName(), TEXT("In"));
	Inputs->bHidden = true;
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, UMontageGraphPinNames::PinCategory_Edge, FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

void UMontageGraphEdNodeEdge::PrepareForCopying()
{
	RuntimeEdge->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

FText UMontageGraphEdNodeEdge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// if (RuntimeEdge)
	// {
	// 	return RuntimeEdge->GetNodeTitle();
	// }
	return FText();
}

void UMontageGraphEdNodeEdge::PinConnectionListChanged(UEdGraphPin* Pin)
{
	if (Pin->LinkedTo.Num() == 0)
	{
		// Transitions must always have an input and output connection
		Modify();

		// Our parent graph will have our graph in SubGraphs so needs to be modified to record that.
		if (UEdGraph* ParentGraph = GetGraph())
		{
			ParentGraph->Modify();
		}

		DestroyNode();
	}
}

void UMontageGraphEdNodeEdge::UpdateCachedIcon()
{
	if (!RuntimeEdge)
	{
		MG_ERROR(Error, TEXT("UMontageGraphEdNodeEdge::UpdateCachedIcon - Invalid Runtime Edge"))
		CachedIcon = nullptr;
		return;
	}
}

UTexture2D* UMontageGraphEdNodeEdge::GetCachedIconTexture() const
{
	return !CachedIcon.IsNull() ? CachedIcon.LoadSynchronous() : nullptr;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UMontageGraphEdNodeEdge::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	const FName MemberPropertyName = (Event.MemberProperty != nullptr) ? Event.MemberProperty->GetFName() : NAME_None;
	const FName PropertyName = (Event.Property != nullptr) ? Event.Property->GetFName() : NAME_None;

	// if (Object == RuntimeEdge && PropertyName == GET_MEMBER_NAME_CHECKED(UHBMontageGraphEdge, TransitionInput))
	// {
	// 	UpdateCachedIcon();
	// }
}
