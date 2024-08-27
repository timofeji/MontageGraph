
#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UActionGraphEdNodeEdge;

class ACTIONGRAPHEDITOR_API SHBActionGraphEdge : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SHBActionGraphEdge) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UActionGraphEdNodeEdge* InNode);

	//~ Begin SGraphNode interface
	virtual bool RequiresSecondPassLayout() const override;
	virtual void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& InNodeToWidgetLookup) const override;
	virtual void UpdateGraphNode() override;
	//~ End SGraphNode interface

	// Calculate position for multiple nodes to be placed between a start and end point, by providing this nodes index and max expected nodes
	void PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

protected:
	FSlateColor GetEdgeColor() const;

	const FSlateBrush* GetEdgeImage() const;

	EVisibility GetEdgeImageVisibility() const;
	EVisibility GetEdgeTitleVisibility() const;
private:

	/** Cached version of GraphNode being observed by this widget with correct type */
	UActionGraphEdNodeEdge* CachedGraphEdge = nullptr;
};
