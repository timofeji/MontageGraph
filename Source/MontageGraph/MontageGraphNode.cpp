// Created by Timofej Jermolaev, All rights reserved . 


#include "MontageGraphNode.h"


#define LOCTEXT_NAMESPACE "MontageGraphNode"

FText UMontageGraphNode::GetNodeTitle() const
{
	FText DefaultText = LOCTEXT("DefaultNodeTitle", "Montage Graph Node");
	return NodeTitle.IsEmpty() ? DefaultText : NodeTitle;
}

UMontageGraphEdge* UMontageGraphNode::GetEdge(UMontageGraphNode* ChildNode)
{
	return Edges.Contains(ChildNode) ? Edges.FindChecked(ChildNode) : nullptr;
}

bool UMontageGraphNode::IsLeafNode() const
{
	return ChildrenNodes.Num() == 0;
}


bool UMontageGraphNode::IsHBActionSubclassedInBlueprint() const
{
	UClass* NodeClass = GetClass();
	if (!NodeClass)
	{
		return false;
	}

	return !NodeClass->IsNative();
}
#if WITH_EDITOR
bool UMontageGraphNode::IsNameEditable() const
{
	return true;
}

void UMontageGraphNode::SetNodeTitle(const FText& NewTitle)
{
	NodeTitle = NewTitle;
}

bool UMontageGraphNode::CanCreateConnection(UMontageGraphNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UMontageGraphNode::CanCreateConnectionTo(UMontageGraphNode* Other, const int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	// if (ChildrenLimitType == EACENodeLimit::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	// {
	// 	ErrorMessage = FText::FromString("Children limit exceeded");
	// 	return false;
	// }

	return CanCreateConnection(Other, ErrorMessage);
}

bool UMontageGraphNode::CanCreateConnectionFrom(UMontageGraphNode* Other, const int32 NumberOfParentNodes, FText& ErrorMessage)
{
	return CanCreateConnection(Other, ErrorMessage);
}

#endif

#undef LOCTEXT_NAMESPACE
