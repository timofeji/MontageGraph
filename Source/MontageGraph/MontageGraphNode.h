// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MontageGraphNode.generated.h"

class UMontageGraphEdge;
/**
 * 
 */
UCLASS()
class MONTAGEGRAPH_API UMontageGraphNode : public UObject
{
	GENERATED_BODY()

public:
	// TODO ~Tim: Soft pointers for anim assets and other relevant assets (particles / sounds ?)

	// TODO ~Tim: Rework debug system to use BP debugger
	
	

	/** When not empty, will draw title with specified value instead of using Animation Asset name (Montage or Sequence) */
	FText NodeTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TArray<UMontageGraphNode*> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TArray<UMontageGraphNode*> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TMap<UMontageGraphNode*, UMontageGraphEdge*> Edges;
	
	

	/** Simply returns the value of NodeTitle, and is overridden in child classes to fallback to animation asset name if NodeTitle is empty. */
	virtual FText GetNodeTitle() const;

	UFUNCTION(BlueprintCallable, Category = "MontageGraph")
	virtual UMontageGraphEdge* GetEdge(UMontageGraphNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "MontageGraph")
	virtual bool IsLeafNode() const;

	/** Checks all child nodes and the edge they're connected with and return the */
	// virtual UMontageGraphEdge* GetEdgeWithInput(UInputAction* InputAction);

	/** Returns whether the class is not a direct descendant of native node classes, indicating it's implemented in BP */
	bool IsHBActionSubclassedInBlueprint() const;
	

#if WITH_EDITORONLY_DATA
	/**
	 * ContextMenuName is used in MontageGraph to generate context menu items (upon right click in the graph to add new nodes)
	 *
	 * Split up ContextMenuName by "|" to create a top category if there is more than one level.
	 *
	 * You can leave this empty to exclude this class from being considered when MontageGraph generates a context menu.
	 */
	// UPROPERTY(EditDefaultsOnly, Category = "MontageGraph", meta = (HideInHBActioGraphDetails))
	FText ContextMenuName;
	
	// /** Set it to false to prevent context menu in graph to include the BP Class name */
	// UPROPERTY(EditDefaultsOnly, Category = "MontageGraph", meta = (HideInHBActioGraphDetailsPanel))
	bool bIncludeClassNameInContextMenu = true;

	// UPROPERTY(EditDefaultsOnly, Category = "MontageGraph" , meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == ENodeLimit::Limited", EditConditionHides))
	int32 ChildrenLimit = 0;
#endif

#if WITH_EDITOR
	virtual bool IsNameEditable() const;

	virtual void SetNodeTitle(const FText& NewTitle);

	virtual FLinearColor GetBackgroundColor() const { return FLinearColor::White; };

	virtual bool CanCreateConnection(UMontageGraphNode* Other, FText& ErrorMessage);
	virtual bool CanCreateConnectionTo(UMontageGraphNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UMontageGraphNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);
#endif
};
