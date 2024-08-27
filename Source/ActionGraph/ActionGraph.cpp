// Created by Timofej Jermolaev, All rights reserved . 

#include "ActionGraph.h"

UActionGraph::UActionGraph()
{
	// NodeType = UHBActionGraphNode::StaticClass();
	// EdgeType = UHBActionGraphEdge::StaticClass();
}

void UActionGraph::ClearGraph()
{
	for (UActionGraphNode* Node : AllNodes)
	{
		if (Node)
		{
			Node->ParentNodes.Empty();
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	EntryNodes.Empty();
	AllNodes.Empty();
}

