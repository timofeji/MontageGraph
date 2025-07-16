// Created by Timofej Jermolaev, All rights reserved . 


#include "SMGNode_Selector.h"
#include "GraphEditorSettings.h"
#include "MontageGraphEditorStyle.h"
#include "IDocumentation.h"
#include "SCommentBubble.h"
#include "SGraphPanel.h"
#include "Graph/EdNodes/MGEdNode_Selector.h"

#define LOCTEXT_NAMESPACE "SMGNode_Selector"

void SMGNode_Selector::Construct(const FArguments& InArgs, UMGEdNode_Selector* InNode)
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

TSharedPtr<SToolTip> SMGNode_Selector::GetComplexTooltip()
{
	UMGEdNode_Selector* StateNode = CastChecked<UMGEdNode_Selector>(GraphNode);

	return SNew(SToolTip)
		[
			SNew(SVerticalBox)

			// TODO: Look at this implementation for custom tooltip for montage / sequence nodes where we would render anim in a preview persona window,
			// alongside documentation and / or additional properties

			// +SVerticalBox::Slot()
			// .AutoHeight()
			// [
			// 	// Create the tooltip preview, ensure to disable state overlays to stop
			// 	// PIE and read-only borders obscuring the graph
			// 	SNew(SGraphPreviewer, StateNode->GetBoundGraph())
			// 	.CornerOverlayText(this, &SMontageGraphNodeSelector::GetPreviewCornerText)
			// 	.ShowGraphStateOverlay(false)
			// ]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 5.0f, 0.0f, 0.0f))
			[
				IDocumentation::Get()->CreateToolTip(FText::FromString("Documentation"), nullptr,
				                                     StateNode->GetDocumentationLink(),
				                                     StateNode->GetDocumentationExcerptName())
			]

		];
}


FSlateColor SMGNode_Selector::GetBorderBackgroundColor() const
{
	// Intentionally basic (might consider adding debug states here)
	constexpr FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
	FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
	FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);

	return InactiveStateColor;
}

const FSlateBrush* SMGNode_Selector::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("Graph.SelectorNode.Icon"));
}

#undef LOCTEXT_NAMESPACE
