// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MGNode.generated.h"

class UMGEdge;
/**
 * 
 */
UCLASS()
class MONTAGEGRAPH_API UMGNode : public UObject
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	int ID;

	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TArray<UMGNode*> ChildrenNodes;
	
	UPROPERTY(BlueprintReadOnly, Category = "Montage Graph")
	TArray<UMGEdge*> Edges;

	
	virtual FText GetNodeTitle() const;
	FText NodeTitle;

#if WITH_EDITORONLY_DATA
	/** When not empty, will draw title with specified value instead of using Animation Asset name (Montage or Sequence) */
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
	
	TArray<UMGNode*> ParentNodes;
#endif

#if WITH_EDITOR

	virtual bool IsNameEditable() const;

	/** Simply returns the value of NodeTitle, and is overridden in child classes to fallback to animation asset name if NodeTitle is empty. */
	virtual void SetNodeTitle(const FText& NewTitle);

	virtual FLinearColor GetBackgroundColor() const { return FLinearColor::White; };

	virtual bool CanCreateConnection(UMGNode* Other, FText& ErrorMessage);
	virtual bool CanCreateConnectionTo(UMGNode* Other, int32 NumberOfChildrenNodes, FText& ErrorMessage);
	virtual bool CanCreateConnectionFrom(UMGNode* Other, int32 NumberOfParentNodes, FText& ErrorMessage);
#endif
};
