

#include "MGEdNode_Edge.h"

#include "MontageGraphEditorTypes.h"

#include "MGEdNode.h"
#include "MGEdNode_Selector.h"
#include "MontageGraph/Nodes/MGEdge.h"

UMGEdNode_Edge::UMGEdNode_Edge()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UMGEdNode_Edge::OnObjectPropertyChanged);
}

UMGEdNode_Edge::~UMGEdNode_Edge()
{
    FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}

FLinearColor UMGEdNode_Edge::GetEdgeColor()
{
	return EdgeColor;
}

void UMGEdNode_Edge::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FProperty* ChangedProperty = PropertyChangedEvent.Property;

	if (ChangedProperty)
	{
		// Evaluation mode
		if (ChangedProperty->GetFName() == GET_MEMBER_NAME_STRING_CHECKED(UMGEdNode_Edge, TransitionType))
		{
			RuntimeEdge->ClearFlags(RF_Public | RF_Standalone);
			RuntimeEdge->MarkAsGarbage();

			switch (TransitionType)
			{
			case ELinkTransitionRule::GameplayEffectQuery:
				RuntimeEdge = NewObject<UMGEdge_GameplayEffectQuery>(this, UMGEdge_GameplayEffectQuery::StaticClass());
				break;
			case ELinkTransitionRule::TagQuery:
				RuntimeEdge = NewObject<UMGEdge_TagQuery>(this, UMGEdge_TagQuery::StaticClass());
				break;
			case ELinkTransitionRule::StackCount:
				RuntimeEdge = NewObject<UMGEdge_StackCount>(this, UMGEdge_StackCount::StaticClass());
				break;
			default:
				RuntimeEdge = NewObject<UMGEdge_Combo>(this, UMGEdge_Combo::StaticClass());
			}

			ReconstructNode();
		}
	}
	



}

void UMGEdNode_Edge::SetRuntimeEdge(UMGEdge* InEdge)
{
	RuntimeEdge = InEdge;
	UpdateCachedIcon();
}

void UMGEdNode_Edge::CreateConnections(const UMGEdNode* Start, const UMGEdNode* End)
{
	CreateBaseConnections(Start, End);
}

void UMGEdNode_Edge::CreateBaseConnections(const UMGEdNode* Start, const UMGEdNode* End)
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

UMGEdNode* UMGEdNode_Edge::GetStartNode() const
{
	if (Pins[0]->LinkedTo.Num() > 0)
	{
		return Cast<UMGEdNode>(Pins[0]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

UMGEdNode* UMGEdNode_Edge::GetEndNode() const
{
	if (Pins[1]->LinkedTo.Num() > 0)
	{
		return Cast<UMGEdNode>(Pins[1]->LinkedTo[0]->GetOwningNode());
	}

	return nullptr;
}

void UMGEdNode_Edge::AllocateDefaultPins()
{
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, UMontageGraphPinNames::PinCategory_Edge, FName(), TEXT("In"));
	Inputs->bHidden = true;
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, UMontageGraphPinNames::PinCategory_Edge, FName(), TEXT("Out"));
	Outputs->bHidden = true;
}

void UMGEdNode_Edge::PrepareForCopying()
{
	RuntimeEdge->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

FText UMGEdNode_Edge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// if (RuntimeEdge)
	// {
	// 	return RuntimeEdge->GetNodeTitle();
	// }
	return FText();
}

void UMGEdNode_Edge::PinConnectionListChanged(UEdGraphPin* Pin)
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

void UMGEdNode_Edge::UpdateCachedIcon()
{
	if (!RuntimeEdge)
	{
		CachedIcon = nullptr;
		return;
	}
}

UTexture2D* UMGEdNode_Edge::GetCachedIconTexture() const
{
	return !CachedIcon.IsNull() ? CachedIcon.LoadSynchronous() : nullptr;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UMGEdNode_Edge::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	const FName MemberPropertyName = (Event.MemberProperty != nullptr) ? Event.MemberProperty->GetFName() : NAME_None;
	const FName PropertyName = (Event.Property != nullptr) ? Event.Property->GetFName() : NAME_None;

	// if (Object == RuntimeEdge && PropertyName == GET_MEMBER_NAME_CHECKED(UMontageGraphEdge, TransitionInput))
	// {
	// 	UpdateCachedIcon();
	// }
}
