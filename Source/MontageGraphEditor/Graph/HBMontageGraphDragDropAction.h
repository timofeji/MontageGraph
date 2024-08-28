// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "GraphEditorDragDropAction.h"

class UMontageGraphEdNode;
struct FGraphPinHandle;

/** Customized version of FDragConnection (mainly because it is not exposed to other modules) */
class MONTAGEGRAPHEDITOR_API FHBMontageGraphDragDropAction : public FGraphEditorDragDropAction
{
public:
	DRAG_DROP_OPERATOR_TYPE(FACEGraphDragAction, FGraphEditorDragDropAction)

	FHBMontageGraphDragDropAction(const TSharedRef<SGraphPanel>& GraphPanel, TArray<FGraphPinHandle>& DraggedPins);
	static TSharedRef<FHBMontageGraphDragDropAction> New(const TSharedRef<SGraphPanel>& GraphPanel, TArray<FGraphPinHandle>& InStartingPins);

	//~ FDragDropOperation interface
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	//~ End FDragDropOperation interface

	//~ FGraphEditorDragDropAction interface
	virtual void HoverTargetChanged() override;
	virtual void OnDragged(const FDragDropEvent& DragDropEvent) override;
	virtual FReply DroppedOnPin(FVector2D ScreenPosition, FVector2D GraphPosition) override;
	virtual FReply DroppedOnNode(FVector2D ScreenPosition, FVector2D GraphPosition) override;
	virtual FReply DroppedOnPanel(const TSharedRef<SWidget>& Panel, FVector2D ScreenPosition, FVector2D GraphPosition, UEdGraph& Graph) override;
	//~ End FGraphEditorDragDropAction interface

	/*
	 *	Get valid starting pin list by checking validity of graph pins in the StartPins list.
	 *
	 *	This check helps to prevent processing graph pins which are outdated.
	 */
	virtual void ValidateGraphPinList(OUT TArray<UEdGraphPin*>& OutValidPins);

protected:
	TSharedPtr<SGraphPanel> GraphPanel;
	TArray<FGraphPinHandle> DraggingPins;

	/** Offset information for the decorator widget */
	FVector2D DecoratorAdjust;

private:

	void AppendConnectionResponsesForHoveredPin(const UEdGraphPin* TargetPin, TArray<FPinConnectionResponse>& OutResponses);
	void AppendConnectionResponsesForHoveredNode(const UMontageGraphEdNode* TargetNode, TArray<FPinConnectionResponse>& OutResponses);
	void AppendConnectionResponsesForHoveredGraph(const UEdGraph* HoverGraph, TArray<FPinConnectionResponse>& OutResponses);
};
