
#include "SHBActionGraphEdge.h"

#include "ConnectionDrawingPolicy.h"
#include "ActionGraph/ActionGraphEdge.h"
#include "ActionGraphEditor/Graph/Nodes/HBActionGraphEdNodeEdge.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SHBActionGraphEdge"

void SHBActionGraphEdge::Construct(const FArguments& InArgs, UActionGraphEdNodeEdge* InNode)
{
	check(InNode)
	GraphNode = InNode;
	CachedGraphEdge = InNode;
	UpdateGraphNode();
}

bool SHBActionGraphEdge::RequiresSecondPassLayout() const
{
	return true;
}

void SHBActionGraphEdge::PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& InNodeToWidgetLookup) const
{
	// CG_EDITOR_LOG(Verbose, TEXT("SHBActionGraphEdge::PerformSecondPassLayout"));
	const UActionGraphEdNodeEdge* EdgeNode = CastChecked<UActionGraphEdNodeEdge>(GraphNode);

	FGeometry StartGeom;
	FGeometry EndGeom;

	int32 NodeIndex = 0;
	int32 MaxNodes = 1;

	// UActionGraphEdNode* Start = EdgeNode->GetStartNodeAsGraphNode();
	UActionGraphEdNode* Start = EdgeNode->GetStartNodeAsBase();
	UActionGraphEdNode* End = EdgeNode->GetEndNode();
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

void SHBActionGraphEdge::UpdateGraphNode()
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
				.ColorAndOpacity(this, &SHBActionGraphEdge::GetEdgeColor)
			]
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(this, &SHBActionGraphEdge::GetEdgeImage)
				.Visibility(this, &SHBActionGraphEdge::GetEdgeImageVisibility)
			]

			+ SOverlay::Slot()
			.Padding(FMargin(4.0f, 4.0f, 4.0f, 4.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SAssignNew(InlineEditableText, SInlineEditableTextBlock)
					.ColorAndOpacity(FLinearColor::Black)
					.Visibility(this, &SHBActionGraphEdge::GetEdgeTitleVisibility)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
					.OnTextCommitted(this, &SHBActionGraphEdge::OnNameTextCommited)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					NodeTitle.ToSharedRef()
				]

			]
		];
}

void SHBActionGraphEdge::PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const
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

	const FVector2D NewCenter = StartAnchorPoint + (0.5f * DeltaPos) + (Height * Normal);

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

void SHBActionGraphEdge::OnNameTextCommited(const FText& InText, const ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	UActionGraphEdNodeEdge* MyNode = CastChecked<UActionGraphEdNodeEdge>(GraphNode);

	if (MyNode != nullptr && MyNode->RuntimeEdge != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("ACEGraphEditorRenameEdge", "ACE Graph Editor: Rename Edge"));
		MyNode->Modify();
		MyNode->RuntimeEdge->SetNodeTitle(InText);
		UpdateGraphNode();
	}
}

FSlateColor SHBActionGraphEdge::GetEdgeColor() const
{
	UActionGraphEdNodeEdge* EdgeNode = CastChecked<UActionGraphEdNodeEdge>(GraphNode);
	if (EdgeNode != nullptr && EdgeNode->RuntimeEdge != nullptr)
	{
		return EdgeNode->RuntimeEdge->GetEdgeColour();
	}

	return FLinearColor(0.4f, 0.9f, 0.9f, 1.0f);
}

const FSlateBrush* SHBActionGraphEdge::GetEdgeImage() const
{
	const FSlateBrush* DefaultBrush = FAppStyle::GetBrush("Graph.TransitionNode.Icon");

	if (!CachedGraphEdge)
	{
		return DefaultBrush;
	}

	if (UTexture2D* Texture = CachedGraphEdge->GetCachedIconTexture())
	{
		FSlateBrush* Brush = new FSlateBrush();
		Brush->SetResourceObject(Texture);
		// // const float IconSize = FHBActionGraphUtils::GetPluginProjectSettings()->IconSize;
		// Brush->ImageSize = FVector2D(IconSize);
		return Brush;
	}

	return DefaultBrush;
}

EVisibility SHBActionGraphEdge::GetEdgeImageVisibility() const
{
	return EVisibility::Visible;
}

EVisibility SHBActionGraphEdge::GetEdgeTitleVisibility() const
{
	return EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
