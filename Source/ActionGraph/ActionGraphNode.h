// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActionGraphNode.generated.h"

class UActionGraphEdge;
/**
 * 
 */
UCLASS()
class ACTIONGRAPH_API UActionGraphNode : public UObject
{
	GENERATED_BODY()

public:
	// TODO ~Tim: Soft pointers for anim assets and other relevant assets (particles / sounds ?)

	// TODO ~Tim: Rework debug system to use BP debugger
	
	

	/** When not empty, will draw title with specified value instead of using Animation Asset name (Montage or Sequence) */
	FText NodeTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Action Graph")
	TArray<UActionGraphNode*> ChildrenNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Action Graph")
	TArray<UActionGraphNode*> ParentNodes;

	UPROPERTY(BlueprintReadOnly, Category = "Action Graph")
	TMap<UActionGraphNode*, UActionGraphEdge*> Edges;
	
	

	/** Simply returns the value of NodeTitle, and is overridden in child classes to fallback to animation asset name if NodeTitle is empty. */
	virtual FText GetNodeTitle() const;

	UFUNCTION(BlueprintCallable, Category = "HBActionGraph")
	virtual UActionGraphEdge* GetEdge(UActionGraphNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "HBActionGraph")
	virtual bool IsLeafNode() const;

	/** Checks all child nodes and the edge they're connected with and return the */
	// virtual UHBActionGraphEdge* GetEdgeWithInput(UInputAction* InputAction);

	/** Returns whether the class is not a direct descendant of native node classes, indicating it's implemented in BP */
	bool IsHBActionSubclassedInBlueprint() const;
	

#if WITH_EDITORONLY_DATA
	/**
	 * ContextMenuName is used in HBActionGraph to generate context menu items (upon right click in the graph to add new nodes)
	 *
	 * Split up ContextMenuName by "|" to create a top category if there is more than one level.
	 *
	 * You can leave this empty to exclude this class from being considered when HBActionGraph generates a context menu.
	 */
	// UPROPERTY(EditDefaultsOnly, Category = "HBActionGraph", meta = (HideInHBActioGraphDetails))
	FText ContextMenuName;
	
	// /** Set it to false to prevent context menu in graph to include the BP Class name */
	// UPROPERTY(EditDefaultsOnly, Category = "HBActionGraph", meta = (HideInHBActioGraphDetailsPanel))
	bool bIncludeClassNameInContextMenu = true;

	// UPROPERTY(EditDefaultsOnly, Category = "HBActionGraph" , meta = (ClampMin = "0", EditCondition = "ChildrenLimitType == ENodeLimit::Limited", EditConditionHides))
	int32 ChildrenLimit = 0;
#endif

#if WITH_EDITOR
	virtual bool IsNameEditable() const;

	virtual void SetNodeTitle(const FText& NewTitle);

	virtual FLinearColor GetBackgroundColor() const { return FLinearColor::White; };

	virtual bool CanCreateConnection(UActionGraphNode* Other, FText& ErrorMessage);
	virtual bool CanCreateConnectionTo(UActionGraphNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UActionGraphNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);
#endif
};
