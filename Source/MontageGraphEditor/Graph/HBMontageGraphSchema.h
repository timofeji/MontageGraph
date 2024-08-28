// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "HBMontageGraphSchema.generated.h"

class UMontageGraphEdNodeEntry;
class UMontageGraphNode;
class UMontageGraphEdNode;
class UMontageGraphEdNodeConduit;
class UMontageGraphEdNodeEdge;
class UMontageGraphEdNodeMontage;

/** Action to add a comment to the graph */
USTRUCT()
struct FHBMontageGraphSchemaAction_AddComment : public FEdGraphSchemaAction
{
	GENERATED_BODY()

	FHBMontageGraphSchemaAction_AddComment() : FEdGraphSchemaAction() {}
	FHBMontageGraphSchemaAction_AddComment(FText InDescription, FText InToolTip)
		: FEdGraphSchemaAction(FText(), MoveTemp(InDescription), MoveTemp(InToolTip), 0)
	{
	}

	// FEdGraphSchemaAction interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override final;
	// End of FEdGraphSchemaAction interface
};

/** Action to add a node to the graph */
USTRUCT()
struct MONTAGEGRAPHEDITOR_API FHBMontageGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHBMontageGraphSchemaAction_NewNode(): NodeTemplate(nullptr) {}

	FHBMontageGraphSchemaAction_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UPROPERTY()
	UMontageGraphEdNode* NodeTemplate;
};

/** Action to add transition edge to the graph */
USTRUCT()
struct MONTAGEGRAPHEDITOR_API FHBMontageGraphSchemaAction_NewEdge : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHBMontageGraphSchemaAction_NewEdge(): NodeTemplate(nullptr){}

	FHBMontageGraphSchemaAction_NewEdge(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UPROPERTY()
	UMontageGraphEdNodeEdge* NodeTemplate;
};


/** Action to add a node action to the graph */
USTRUCT()
struct MONTAGEGRAPHEDITOR_API FHBMontageGraphSchemaAction_NewNodeAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHBMontageGraphSchemaAction_NewNodeAction(): NodeTemplate(nullptr) {}

	FHBMontageGraphSchemaAction_NewNodeAction(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UPROPERTY()
	UMontageGraphEdNodeMontage* NodeTemplate;
};

/** Action to add a node conduit to the graph */
USTRUCT()
struct MONTAGEGRAPHEDITOR_API FHBMontageGraphSchemaAction_NewNodeConduit : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHBMontageGraphSchemaAction_NewNodeConduit(): NodeTemplate(nullptr) {}

	FHBMontageGraphSchemaAction_NewNodeConduit(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UPROPERTY()
	UMontageGraphEdNodeConduit* NodeTemplate;
};

/** Action to add a node conduit to the graph */
USTRUCT()
struct MONTAGEGRAPHEDITOR_API FHBMontageGraphSchemaAction_NewEntryNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHBMontageGraphSchemaAction_NewEntryNode(): NodeTemplate(nullptr) {}

	FHBMontageGraphSchemaAction_NewEntryNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UPROPERTY()
	UMontageGraphEdNodeEntry* NodeTemplate;
};

/** Action to auto arrange the graph (based off BehaviorTree one) */
USTRUCT()
struct FHBMontageGraphSchemaAction_AutoArrangeVertical : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHBMontageGraphSchemaAction_AutoArrangeVertical()
		: FEdGraphSchemaAction() {}

	FHBMontageGraphSchemaAction_AutoArrangeVertical(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};

/** Action to auto arrange the graph (based off BehaviorTree one). This ones tries to layout the graph from left to right */
USTRUCT()
struct FHBMontageGraphSchemaAction_AutoArrangeHorizontal : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FHBMontageGraphSchemaAction_AutoArrangeHorizontal()
		: FEdGraphSchemaAction() {}

	FHBMontageGraphSchemaAction_AutoArrangeHorizontal(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};

UCLASS(MinimalAPI)
class UHBMontageGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:

	//~ UEdGraphSchema interface
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual EGraphType GetGraphType(const UEdGraph* TestEdGraph) const override;
	/** Get all actions that can be performed when right clicking on a graph or drag-releasing on a graph from a pin */
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	/** Gets actions that should be added to the right-click context menu for a node or pin */
	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const override;
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotification) const override;
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
	virtual UEdGraphPin* DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const override;
	virtual bool SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const override;
	virtual bool IsCacheVisualizationOutOfDate(int32 InVisualizationCacheID) const override;
	virtual int32 GetCurrentVisualizationCacheID() const override;
	virtual void ForceVisualizationCacheClear() const override;
	virtual void GetGraphDisplayInformation(const UEdGraph& Graph, FGraphDisplayInfo& DisplayInfo) const override;
	virtual void DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const override;
	virtual void DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const override;
	virtual void DroppedAssetsOnPin(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphPin* Pin) const override;
	virtual void GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const override;
	virtual void GetAssetsPinHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphPin* HoverPin, FString& OutTooltipText, bool& OutOkIcon) const override;
	virtual void GetAssetsNodeHoverMessage(const TArray<FAssetData>& Assets, const UEdGraphNode* HoverNode, FString& OutTooltipText, bool& OutOkIcon) const override;
	virtual bool CanDuplicateGraph(UEdGraph* InSourceGraph) const override;
	virtual int32 GetNodeSelectionCount(const UEdGraph* Graph) const override;
	virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	//~ End UEdGraphSchema interface

	/** Spawn the correct node in the HBMontageGraph using the given AnimationAsset at the supplied location */
	static void SpawnNodeFromAsset(UAnimationAsset* Asset, const FVector2D& GraphPosition, UEdGraph* Graph, UEdGraphPin* PinIfAvailable);

	/** Helper to return correct Node Handler depending on provided Animation Asset (can be Montages or Sequences) */
	static UClass* GetNodeClassForAnimAsset(const UAnimationAsset* Asset, const UEdGraph* Graph);

	/** Helper to return correct Runtime Node depending on provided Animation Asset (can be Montages or Sequences) */
	static UClass* GetRuntimeClassForAnimAsset(const UAnimationAsset* Asset, const UEdGraph* Graph);

	/** See if a particular anim InGraphNode can play a particular anim AssetClass */
	static bool SupportNodeClassForAsset(UClass* AssetClass, const UEdGraphNode* InGraphNode);

private:
	static int32 CurrentCacheRefreshID;

	void CreateEdgeConnection(UEdGraphPin* PinA, UEdGraphPin* PinB, const UMontageGraphEdNode* OwningNodeA, const UMontageGraphEdNode* OwningNodeB) const;
	void CreateAndAddActionToContextMenu(FGraphContextMenuBuilder& ContextMenuBuilder, TSubclassOf<UMontageGraphNode> NodeType) const;

	static bool IsHoverPinMatching(const UEdGraphPin* InHoverPin);

	static FVector2D GetFixedOffsetFromPin(const UEdGraphPin* InHoverPin);
};
