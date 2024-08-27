// Created by Timofej Jermolaev, All rights reserved . 


#include "ActionGraphNode.h"


#define LOCTEXT_NAMESPACE "HBActionGraphNode"

FText UActionGraphNode::GetNodeTitle() const
{
	FText DefaultText = LOCTEXT("DefaultNodeTitle", "Action Graph Node");
	return NodeTitle.IsEmpty() ? DefaultText : NodeTitle;
}

UActionGraphEdge* UActionGraphNode::GetEdge(UActionGraphNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}

bool UActionGraphNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}


bool UActionGraphNode::IsHBActionSubclassedInBlueprint() const
{
	UClass* NodeClass = GetClass();
	if (!NodeClass)
	{
		return false;
	}

	return !NodeClass->IsNative();
}
#if WITH_EDITOR
bool UActionGraphNode::IsNameEditable() const
{
	return true;
}

void UActionGraphNode::SetNodeTitle(const FText& NewTitle)
{
	NodeTitle = NewTitle;
}

bool UActionGraphNode::CanCreateConnection(UActionGraphNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UActionGraphNode::CanCreateConnectionTo(UActionGraphNode* Other, const int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	// if (ChildrenLimitType == EACENodeLimit::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	// {
	// 	ErrorMessage = FText::FromString("Children limit exceeded");
	// 	return false;
	// }

	return CanCreateConnection(Other, ErrorMessage);
}

bool UActionGraphNode::CanCreateConnectionFrom(UActionGraphNode* Other, const int32 NumberOfParentNodes, FText& ErrorMessage)
{
	return CanCreateConnection(Other, ErrorMessage);
}

#endif

#undef LOCTEXT_NAMESPACE
