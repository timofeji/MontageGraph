// Created by Timofej Jermolaev, All rights reserved . 


#include "MontageGraphNodePanelFactory.h"

#include "MontageGraphEditorSettings.h"
#include "Slate/SMontageGraphEdge.h"
#include "Slate/SGraphNodeAction.h"
#include "Slate/SMontageGraphNodeSelector.h"
#include "Slate/SMontageGraphNodeEntry.h"
#include "Nodes\MontageGraphEdNodeEdge.h"
#include "Nodes\MontageGraphEdNodeEntry.h"
#include "Nodes/MontageGraphEdNodeMontage.h"

TSharedPtr<SGraphNode> FMontageGraphNodePanelFactory::CreateNode(UEdGraphNode* Node) const
{

	if (UMontageGraphEdNodeEdge* GraphEdge = Cast<UMontageGraphEdNodeEdge>(Node))
	{
		return SNew(SMontageGraphEdge, GraphEdge);
	}

	if (UMontageGraphEdNodeEntry* EntryNode = Cast<UMontageGraphEdNodeEntry>(Node))
	{
		return SNew(SMontageGraphNodeEntry, EntryNode);
	}

	// if (UMontageGraphEdNodeSelector* SelectorNode = Cast<UMontageGraphEdNodeSelector>(Node))
	// {
	// 	return SNew(SMontageGraphNodeSelector, SelectorNode);
	// }

	if (UMontageGraphEdNodeMontage* GraphNode = Cast<UMontageGraphEdNodeMontage>(Node))
	{
		const UMontageGraphEditorSettings* Settings = GetDefault<UMontageGraphEditorSettings>();
	
		// const bool bDrawVerticalPins = Settings->DrawPinTypes == EComboGraphDrawPinTypes::Both || Settings->DrawPinTypes == EComboGraphDrawPinTypes::Vertical;
		// const bool bDrawHorizontalPins = Settings->DrawPinTypes == EComboGraphDrawPinTypes::Both || Settings->DrawPinTypes == EComboGraphDrawPinTypes::Horizontal;
	
		return SNew(SGraphNodeAction, GraphNode)
			.PinSize(Settings->PinSize)
			.PinPadding(Settings->PinPadding)
			.DrawVerticalPins(true)
			.DrawHorizontalPins(true)
			.ContentPadding(Settings->ContentMargin)
			.ContentInternalPadding(Settings->ContentInternalPadding);
	}

	return nullptr;
}
