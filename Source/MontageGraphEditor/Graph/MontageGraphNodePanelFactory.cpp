// Created by Timofej Jermolaev, All rights reserved . 


#include "MontageGraphNodePanelFactory.h"

#include "MontageGraphEditorSettings.h"
#include "Slate/SHBMontageGraphEdge.h"
#include "Slate/SGraphNodeAction.h"
#include "Slate/SHBGraphNodeSelector.h"
#include "Slate/SHBMontageGraphNodeEntry.h"
#include "Nodes/HBMontageGraphEdNodeEdge.h"
#include "Nodes/HBMontageGraphEdNodeEntry.h"
#include "Nodes/MontageGraphEdNodeMontage.h"

TSharedPtr<SGraphNode> FMontageGraphNodePanelFactory::CreateNode(UEdGraphNode* Node) const
{

	if (UMontageGraphEdNodeEdge* GraphEdge = Cast<UMontageGraphEdNodeEdge>(Node))
	{
		return SNew(SHBMontageGraphEdge, GraphEdge);
	}

	if (UMontageGraphEdNodeEntry* EntryNode = Cast<UMontageGraphEdNodeEntry>(Node))
	{
		return SNew(SHBMontageGraphNodeEntry, EntryNode);
	}

	// if (UMontageGraphEdNodeSelector* SelectorNode = Cast<UMontageGraphEdNodeSelector>(Node))
	// {
	// 	return SNew(SHBGraphNodeSelector, SelectorNode);
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
