
#include "SMGNode_Edge.h"

#include "ConnectionDrawingPolicy.h"
#include "MontageGraphEditorStyle.h"
#include "Graph\EdNodes\MGEdNode_Edge.h"
#include "MontageGraph/Nodes/MGEdge.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SMGNode_Edge"

void SMGNode_Edge::Construct(const FArguments& InArgs, UMGEdNode_Edge* InNode)
{
	check(InNode)
	GraphNode = InNode;
	CachedGraphEdge = InNode;
	UpdateGraphNode();
}

bool SMGNode_Edge::RequiresSecondPassLayout() const
{
	return true;
}

void SMGNode_Edge::PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& InNodeToWidgetLookup) const
{
	const UMGEdNode_Edge* EdgeNode = CastChecked<UMGEdNode_Edge>(GraphNode);

	FGeometry StartGeom;
	FGeometry EndGeom;

	int32 NodeIndex = 0;
	int32 MaxNodes = 1;

	// UMGEdNode* Start = EdgeNode->GetStartNodeAsGraphNode();
	UMGEdNode* Start = EdgeNode->GetStartNode();
	UMGEdNode* End = EdgeNode->GetEndNode();
	if (Start != nullptr && End != nullptr)
	{
		const TSharedRef<SNode>* FromWidgetPtr = InNodeToWidgetLookup.Find(Start);
		const TSharedRef<SNode>* ToWidgetPtr = InNodeToWidgetLookup.Find(End);
		if (FromWidgetPtr && ToWidgetPtr)
		{
			const TSharedRef<SNode>& FromWidget = *FromWidgetPtr;
			const TSharedRef<SNode>& ToWidget = *ToWidgetPtr;

			StartGeom = FGeometry(FVector2D(Start->NodePosX, Start->NodePosY), FVector2D::ZeroVector, FromWidget->GetDesiredSize(), 1.0f);
			EndGeom = FGeometry(FVector2D(End->NodePosX, End->NodePosY), FVector2D::ZeroVector, ToWidget->GetDesiredSize(), 1.0f);
		}
	}

	PositionBetweenTwoNodesWithOffset(StartGeom, EndGeom, NodeIndex, MaxNodes);
}

void SMGNode_Edge::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();

	const TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Graph.TransitionNode.ColorSpill"))
				.ColorAndOpacity(this, &SMGNode_Edge::GetEdgeColor)
			]
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(24.f))
				.Image(this, &SMGNode_Edge::GetEdgeImage)
				.Visibility(this, &SMGNode_Edge::GetEdgeImageVisibility)
			]

			// + SOverlay::Slot()
			// .Padding(FMargin(2.f))
			// [
			// 	SNew(SVerticalBox)
			// 	+ SVerticalBox::Slot()
			// 	.HAlign(HAlign_Center)
			// 	.AutoHeight()
			// 	[
			// 		SAssignNew(InlineEditableText, SInlineEditableTextBlock)
			// 		.ColorAndOpacity(FLinearColor::Black)
			// 		.Visibility(this, &SMGNode_Edge::GetEdgeTitleVisibility)
			// 		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			// 		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
			// 		.OnTextCommitted(this, &SMGNode_Edge::OnNameTextCommited)
			// 	]
			// 	+ SVerticalBox::Slot()
			// 	.AutoHeight()
			// 	[
			// 		NodeTitle.ToSharedRef()
			// 	]
			//
			// ]
		];
}

void SMGNode_Edge::PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const
{
	// Get a reasonable seed point (halfway between the boxes)
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);

	if(StartCenter == EndCenter)
	{
		FVector2D Corner = StartCenter;
		Corner.X += StartGeom.Size.X / 2.f + 10.f;
		Corner.Y += StartGeom.Size.Y / 2.f + 15.f;

		GraphNode->NodePosX = Corner.X;
		GraphNode->NodePosY = Corner.Y;

		return;
	}

	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

	// Find the (approximate) closest points between the two boxes
	const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	// Position ourselves halfway along the connecting line between the nodes, elevated away perpendicular to the direction of the line
	const float Height = 30.0f;

	const FVector2D DesiredNodeSize = GetDesiredSize();

	FVector2D DeltaPos(EndAnchorPoint - StartAnchorPoint);

	if (DeltaPos.IsNearlyZero())
	{
		DeltaPos = FVector2D(10.0f, 0.0f);
	}

	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	const FVector2D NewCenter = StartAnchorPoint + (0.5f * DeltaPos) ;
// + (Height * Normal)
	const FVector2D DeltaNormal = DeltaPos.GetSafeNormal();

	// Calculate node offset in the case of multiple transitions between the same two nodes
	// MultiNodeOffset: the offset where 0 is the center of the transition, -1 is 1 <size of node>
	// towards the PrevStateNode and +1 is 1 <size of node> towards the NextStateNode.

	constexpr float MutliNodeSpace = 0.2f; // Space between multiple transition nodes (in units of <size of node> )
	constexpr float MultiNodeStep = (1.f + MutliNodeSpace); //Step between node centres (Size of node + size of node spacer)

	const float MultiNodeStart = -((MaxNodes - 1) * MultiNodeStep) / 2.f;
	const float MultiNodeOffset = MultiNodeStart + (NodeIndex * MultiNodeStep);

	// Now we need to adjust the new center by the node size, zoom factor and multi node offset
	const FVector2D NewCorner = NewCenter - (0.5f * DesiredNodeSize) + (DeltaNormal * MultiNodeOffset * DesiredNodeSize.Size());

	GraphNode->NodePosX = NewCorner.X;
	GraphNode->NodePosY = NewCorner.Y;
}

void SMGNode_Edge::OnNameTextCommited(const FText& InText, const ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	UMGEdNode_Edge* MyNode = CastChecked<UMGEdNode_Edge>(GraphNode);

	if (MyNode != nullptr && MyNode->RuntimeEdge != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("MontageGraphEditorRenameEdge", "MontageGraph Editor: Rename Edge"));
		MyNode->Modify();
		MyNode->RuntimeEdge->SetNodeTitle(InText);
		UpdateGraphNode();
	}
}

FSlateColor SMGNode_Edge::GetEdgeColor() const
{
	UMGEdNode_Edge* EdgeEdNode = CastChecked<UMGEdNode_Edge>(GraphNode);
	if (EdgeEdNode != nullptr && EdgeEdNode->RuntimeEdge != nullptr)
	{
		return EdgeEdNode->GetEdgeColour();
	}

	return FLinearColor(0.4f, 0.9f, 0.9f, 1.0f);
}

const FSlateBrush* SMGNode_Edge::GetEdgeImage() const
{
	const FSlateBrush* DefaultBrush = FMontageGraphEditorStyle::Get().GetBrush("Graph.Link.Icon");

	if (!CachedGraphEdge)
	{
		return DefaultBrush;
	}

	if (UTexture2D* Texture = CachedGraphEdge->GetCachedIconTexture())
	{
		FSlateBrush* Brush = new FSlateBrush();
		Brush->SetResourceObject(Texture);
		// // const float IconSize = FMontageGraphUtils::GetPluginProjectSettings()->IconSize;
		// Brush->ImageSize = FVector2D(IconSize);
		return Brush;
	}

	return DefaultBrush;
}

EVisibility SMGNode_Edge::GetEdgeImageVisibility() const
{
	return EVisibility::Visible;
}

EVisibility SMGNode_Edge::GetEdgeTitleVisibility() const
{
	return EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
