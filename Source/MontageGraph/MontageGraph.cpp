// Created by Timofej Jermolaev, All rights reserved . 

#include "MontageGraph.h"


UMontageGraph::UMontageGraph()
{
	// NodeType = UMGNode::StaticClass();
	// EdgeType = UMontageGraphEdge::StaticClass();
}

void UMontageGraph::ClearGraph()
{
	for (UMGNode* Node : AllNodes)
	{
		if (Node)
		{
			Node->ChildrenNodes.Empty();
			Node->Edges.Empty();
		}
	}

	RootNodes.Empty();
	AllNodes.Empty();

	GameplayEffects.Empty();
	Montages.Empty();
	CollisionTracers.Empty();
}

void UMontageGraph::SetPreviewMesh(USkeletalMesh* PreviewMesh, bool bMarkAsDirty)
{
	if(bMarkAsDirty)
	{
		Modify();
	}
	PreviewSkeletalMesh = PreviewMesh;
}

USkeletalMesh* UMontageGraph::GetPreviewMesh() const
{
	USkeletalMesh* PreviewMesh = PreviewSkeletalMesh.Get();
	if (!PreviewMesh)
	{
		// if preview mesh isn't loaded, see if we have set
		FSoftObjectPath PreviewMeshStringRef = PreviewSkeletalMesh.ToSoftObjectPath();
		// load it since now is the time to load
		if (!PreviewMeshStringRef.ToString().IsEmpty())
		{
			PreviewMesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr,
			                                                   *PreviewMeshStringRef.ToString(), nullptr, LOAD_None,
			                                                   nullptr));
		}
	}

	return PreviewMesh;
}


