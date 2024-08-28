// Created by Timofej Jermolaev, All rights reserved . 


#include "HBMontageGraphNodePanelFactory.h"

#include "MontageGraphEditorSettings.h"
#include "Slate/SHBMontageGraphEdge.h"
#include "Slate/SHBGraphNodeAction.h"
#include "Slate/SHBGraphNodeConduit.h"
#include "Slate/SHBMontageGraphNodeEntry.h"
#include "Nodes/HBMontageGraphEdNodeConduit.h"
#include "Nodes/HBMontageGraphEdNodeEdge.h"
#include "Nodes/HBMontageGraphEdNodeEntry.h"

TSharedPtr<SGraphNode> FHBMontageGraphNodePanelFactory::CreateNode(UEdGraphNode* Node) const
{

	if (UMontageGraphEdNodeEdge* GraphEdge = Cast<UMontageGraphEdNodeEdge>(Node))
	{
		return SNew(SHBMontageGraphEdge, GraphEdge);
	}

	if (UMontageGraphEdNodeEntry* EntryNode = Cast<UMontageGraphEdNodeEntry>(Node))
	{
		return SNew(SHBMontageGraphNodeEntry, EntryNode);
	}

	if (UMontageGraphEdNodeConduit* ConduitNode = Cast<UMontageGraphEdNodeConduit>(Node))
	{
		return SNew(SHBGraphNodeConduit, ConduitNode);
	}

	if (UMontageGraphEdNode* GraphNode = Cast<UMontageGraphEdNode>(Node))
	{
		const UMontageGraphEditorSettings* Settings = GetDefault<UMontageGraphEditorSettings>();

		// const bool bDrawVerticalPins = Settings->DrawPinTypes == EComboGraphDrawPinTypes::Both || Settings->DrawPinTypes == EComboGraphDrawPinTypes::Vertical;
		// const bool bDrawHorizontalPins = Settings->DrawPinTypes == EComboGraphDrawPinTypes::Both || Settings->DrawPinTypes == EComboGraphDrawPinTypes::Horizontal;

		return SNew(SHBGraphNodeAction, GraphNode)
			.PinSize(Settings->PinSize)
			.PinPadding(Settings->PinPadding)
			.DrawVerticalPins(true)
			.DrawHorizontalPins(true)
			.ContentPadding(Settings->ContentMargin)
			.ContentInternalPadding(Settings->ContentInternalPadding);
	}

	return nullptr;
}
