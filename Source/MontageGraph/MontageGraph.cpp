// Created by Timofej Jermolaev, All rights reserved . 

#include "MontageGraph.h"

UMontageGraph::UMontageGraph()
{
	// NodeType = UHBMontageGraphNode::StaticClass();
	// EdgeType = UHBMontageGraphEdge::StaticClass();
}

void UMontageGraph::ClearGraph()
{
	for (UMontageGraphNode* Node : AllNodes)
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

