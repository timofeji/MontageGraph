// Created by Timofej Jermolaev, All rights reserved . 


#include "MGNode.h"


#define LOCTEXT_NAMESPACE "MontageGraphNode"

FText UMGNode::GetNodeTitle() const
{
	FText DefaultText = LOCTEXT("DefaultNodeTitle", "Montage Graph Node");
	return NodeTitle.IsEmpty() ? DefaultText : NodeTitle;
}

#if WITH_EDITOR


bool UMGNode::IsNameEditable() const
{
	return true;
}

void UMGNode::SetNodeTitle(const FText& NewTitle)
{
	NodeTitle = NewTitle;
}

bool UMGNode::CanCreateConnection(UMGNode* Other, FText& ErrorMessage)
{
	return true;
}

bool UMGNode::CanCreateConnectionTo(UMGNode* Other, const int32 NumberOfChildrenNodes, FText& ErrorMessage)
{
	// if (ChildrenLimitType == EACENodeLimit::Limited && NumberOfChildrenNodes >= ChildrenLimit)
	// {
	// 	ErrorMessage = FText::FromString("Children limit exceeded");
	// 	return false;
	// }

	return CanCreateConnection(Other, ErrorMessage);
}

bool UMGNode::CanCreateConnectionFrom(UMGNode* Other, const int32 NumberOfParentNodes, FText& ErrorMessage)
{
	return CanCreateConnection(Other, ErrorMessage);
}

#endif

#undef LOCTEXT_NAMESPACE
