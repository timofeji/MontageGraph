// Created by Timofej Jermolaev, All rights reserved . 


#include "MontageGraphNodePanelFactory.h"

#include "MontageGraphEditorSettings.h"
#include "EdNodes/MGEdNode_Conduit.h"
#include "Graph/Slate/SMGNode_Edge.h"
#include "Graph/Slate/SMGNode_Montage.h"
#include "Graph/Slate/SMGNode_Selector.h"
#include "Graph/Slate/SMGNode_Entry.h"
#include "EdNodes\MGEdNode_Edge.h"
#include "EdNodes\MGEdNode_Entry.h"
#include "EdNodes/MGEdNode_Montage.h"
#include "EdNodes/MGEdNode_Selector.h"

TSharedPtr<SGraphNode> FMontageGraphNodePanelFactory::CreateNode(UEdGraphNode* Node) const
{

	if (UMGEdNode_Edge* GraphEdge = Cast<UMGEdNode_Edge>(Node))
	{
		return SNew(SMGNode_Edge, GraphEdge);
	}

	if (UMGEdNode_Entry* EntryNode = Cast<UMGEdNode_Entry>(Node))
	{
		return SNew(SMGNode_Entry, EntryNode);
	}

	if (UMGEdNode_Selector* SelectorNode = Cast<UMGEdNode_Selector>(Node))
	{
		return SNew(SMGNode_Selector, SelectorNode);
	}
	
	
	if (UMGEdNode_Conduit* SelectorNode = Cast<UMGEdNode_Conduit>(Node))
	{
		return SNew(SMGNode, SelectorNode);
	}

	if (UMGEdNode_Montage* GraphNode = Cast<UMGEdNode_Montage>(Node))
	{
		const UMontageGraphEditorSettings* Settings = GetDefault<UMontageGraphEditorSettings>();
	
		// const bool bDrawVerticalPins = Settings->DrawPinTypes == EComboGraphDrawPinTypes::Both || Settings->DrawPinTypes == EComboGraphDrawPinTypes::Vertical;
		// const bool bDrawHorizontalPins = Settings->DrawPinTypes == EComboGraphDrawPinTypes::Both || Settings->DrawPinTypes == EComboGraphDrawPinTypes::Horizontal;
	
		return SNew(SMGNode_Montage, GraphNode)
			.PinSize(Settings->PinSize)
			.PinPadding(Settings->PinPadding)
			.DrawVerticalPins(true)
			.DrawHorizontalPins(true)
			.ContentPadding(Settings->ContentMargin)
			.ContentInternalPadding(Settings->ContentInternalPadding);
	}

	return nullptr;
}
