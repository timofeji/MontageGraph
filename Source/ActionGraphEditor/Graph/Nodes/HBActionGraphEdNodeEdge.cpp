

#include "HBActionGraphEdNodeEdge.h"

#include "ActionGraphEditorTypes.h"

#include "ActionGraph/ActionGraphEdge.h"
#include "ActionGraphEdNode.h"
#include "HBActionGraphEdNodeConduit.h"
#include "..\..\ActionGraphEditorLog.h"

UActionGraphEdNodeEdge::UActionGraphEdNodeEdge()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UActionGraphEdNodeEdge::OnObjectPropertyChanged);
}

UActionGraphEdNodeEdge::~UActionGraphEdNodeEdge()
{
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}

void UActionGraphEdNodeEdge::SetRuntimeEdge(UActionGraphEdge* InEdge)
{
	RuntimeEdge = InEdge;
	UpdateCachedIcon();
}

void UActionGraphEdNodeEdge::CreateConnections(const UActionGraphEdNode* Start, const UActionGraphEdNode* End)
{
	CreateBaseConnections(Start, End);
}

void UActionGraphEdNodeEdge::CreateBaseConnections(const UActionGraphEdNode* Start, const UActionGraphEdNode* End)
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

UActionGraphEdNode* UActionGraphEdNodeEdge::GetStartNodeAsBase() const
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UActionGraphEdNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

UActionGraphEdNode* UActionGraphEdNodeEdge::GetStartNodeAsGraphNode() const
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UActionGraphEdNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

UActionGraphEdNodeConduit* UActionGraphEdNodeEdge::GetStartNodeAsConduit() const
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UActionGraphEdNodeConduit>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

UActionGraphEdNode* UActionGraphEdNodeEdge::GetEndNode() const
{
	if (Pins[1]->LinkedTo.Num() > 0)
	{
		return Cast<UActionGraphEdNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

void UActionGraphEdNodeEdge::AllocateDefaultPins()
{
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, UActionGraphPinNames::PinCategory_Edge, FName(), TEXT("In"));
	Inputs->bHidden = true;
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, UActionGraphPinNames::PinCategory_Edge, FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

void UActionGraphEdNodeEdge::PrepareForCopying()
{
	RuntimeEdge->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

FText UActionGraphEdNodeEdge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// if (RuntimeEdge)
	// {
	// 	return RuntimeEdge->GetNodeTitle();
	// }
	return FText();
}

void UActionGraphEdNodeEdge::PinConnectionListChanged(UEdGraphPin* Pin)
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

void UActionGraphEdNodeEdge::UpdateCachedIcon()
{
	if (!RuntimeEdge)
	{
		ACTIONGRAPH_LOG(Error, TEXT("UActionGraphEdNodeEdge::UpdateCachedIcon - Invalid Runtime Edge"))
		CachedIcon = nullptr;
		return;
	}
}

UTexture2D* UActionGraphEdNodeEdge::GetCachedIconTexture() const
{
	return !CachedIcon.IsNull() ? CachedIcon.LoadSynchronous() : nullptr;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UActionGraphEdNodeEdge::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	const FName MemberPropertyName = (Event.MemberProperty != nullptr) ? Event.MemberProperty->GetFName() : NAME_None;
	const FName PropertyName = (Event.Property != nullptr) ? Event.Property->GetFName() : NAME_None;

	// if (Object == RuntimeEdge && PropertyName == GET_MEMBER_NAME_CHECKED(UHBActionGraphEdge, TransitionInput))
	// {
	// 	UpdateCachedIcon();
	// }
}
