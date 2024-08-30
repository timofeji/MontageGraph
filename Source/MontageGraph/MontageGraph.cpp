// Created by Timofej Jermolaev, All rights reserved . 

#include "MontageGraph.h"

UMontageGraph::UMontageGraph()
{
	// NodeType = UMontageGraphNode::StaticClass();
	// EdgeType = UMontageGraphEdge::StaticClass();
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


