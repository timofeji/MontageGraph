// Created by Timofej Jermolaev, All rights reserved . 


#include "HBActionGraphNodePanelFactory.h"

#include "ActionGraphEditorSettings.h"
#include "Slate/SHBActionGraphEdge.h"
#include "Slate/SHBGraphNodeAction.h"
#include "Slate/SHBGraphNodeConduit.h"
#include "Slate/SHBActionGraphNodeEntry.h"
#include "Nodes/HBActionGraphEdNodeConduit.h"
#include "Nodes/HBActionGraphEdNodeEdge.h"
#include "Nodes/HBActionGraphEdNodeEntry.h"

TSharedPtr<SGraphNode> FHBActionGraphNodePanelFactory::CreateNode(UEdGraphNode* Node) const
{

	if (UActionGraphEdNodeEdge* GraphEdge = Cast<UActionGraphEdNodeEdge>(Node))
	{
		return SNew(SHBActionGraphEdge, GraphEdge);
	}

	if (UActionGraphEdNodeEntry* EntryNode = Cast<UActionGraphEdNodeEntry>(Node))
	{
		return SNew(SHBActionGraphNodeEntry, EntryNode);
	}

	if (UActionGraphEdNodeConduit* ConduitNode = Cast<UActionGraphEdNodeConduit>(Node))
	{
		return SNew(SHBGraphNodeConduit, ConduitNode);
	}

	if (UActionGraphEdNode* GraphNode = Cast<UActionGraphEdNode>(Node))
	{
		const UActionGraphEditorSettings* Settings = GetDefault<UActionGraphEditorSettings>();

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
