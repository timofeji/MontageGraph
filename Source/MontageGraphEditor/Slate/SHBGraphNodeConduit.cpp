﻿// Created by Timofej Jermolaev, All rights reserved . 


#include "SHBGraphNodeConduit.h"
#include "GraphEditorSettings.h"
#include "MontageGraphEditorStyles.h"
#include "IDocumentation.h"
#include "SCommentBubble.h"
#include "SGraphPin.h"
#include "Graph/Nodes/HBMontageGraphEdNodeConduit.h"
#include "SHBMontageGraphConduitOutputPin.h"
#include "Widgets/Layout/SScaleBox.h"

#define LOCTEXT_NAMESPACE "SHBMontageGraphNodeConduit"

void SHBGraphNodeConduit::Construct(const FArguments& InArgs, UMontageGraphEdNodeConduit* InNode)
{
	GraphNode = InNode;
	CachedGraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SHBGraphNodeConduit::RefreshOutputPins()
{
	bool bShouldUpdate = true;
	UMontageGraphEdNodeConduit* StateNode = CastChecked<UMontageGraphEdNodeConduit>(GraphNode);

	if (bShouldUpdate) { CreatePinWidgets(); }
}


void SHBGraphNodeConduit::UpdateGraphNode()
{
	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	const FSlateBrush* NodeTypeIcon = GetNameIcon();

	constexpr FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
	TSharedPtr<SErrorText> ErrorText;
	const TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	
	GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SBorder)
		.BorderImage(FMontageGraphEditorStyles::Get().GetBrush("HBEditor.MontageGraph.Node.BackgroundLight"))
		.BorderBackgroundColor(this, &SHBGraphNodeConduit::GetBorderBackgroundColor)
		.Padding(5.f)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(OutNodeBox, SHorizontalBox)
			]
			// + SOverlay::Slot()
			//   .HAlign(HAlign_Fill)
			//   .VAlign(VAlign_Fill)
			//   .Padding(15.0f)
			// [
			// 	SNew(SBorder)
			// 	.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
			// 	.BorderBackgroundColor(TitleShadowColor)
			// 	.HAlign(HAlign_Center)
			// 	.VAlign(VAlign_Center)
			// 	.Visibility(EVisibility::SelfHitTestInvisible)
			// 	[
			// 		SNew(SHorizontalBox)
			// 		+ SHorizontalBox::Slot()
			// 		.AutoWidth()
			// 		[
			// 			// POPUP ERROR MESSAGE @todo ~Tim: Add error text on validation 
			// 			SAssignNew(ErrorText, SErrorText)
			// 			.BackgroundColor(this, &SHBMontageGraphNodeConduit::GetErrorColor)
			// 			.ToolTipText(this, &SHBMontageGraphNodeConduit::GetErrorMsgToolTip)
			// 		]
			// 	]
			// ]

		]
	];


	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];


	// ErrorReporting = ErrorText;
	// ErrorReporting->SetError(ErrorMsg);

	CreatePinWidgets();
}

void SHBGraphNodeConduit::CreatePinWidgets()
{
	OutNodeBox->ClearChildren();
	UMontageGraphEdNode* StateNode = CastChecked<UMontageGraphEdNode>(CachedGraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden && MyPin->Direction == EGPD_Output)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SHBMontageGraphConduitOutputPin, MyPin);
			NewPin->SetOwner(SharedThis(this));
			OutNodeBox->AddSlot()
			          .HAlign(HAlign_Fill)
			          .VAlign(VAlign_Fill)
			          .FillWidth(1.f)
			[
				SNew(SScaleBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.StretchDirection(EStretchDirection::UpOnly)
				.Stretch(EStretch::ScaleToFill)
				[
					SNew(SBox)
					.MinDesiredWidth(25.f)
					.MinDesiredHeight(25.f)
					.MinAspectRatio(1.f)
					[
						NewPin.ToSharedRef()
					]
				]
			];
			OutputPins.Add(NewPin.ToSharedRef());
		}
	}
}

void SHBGraphNodeConduit::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	RightNodeBox->AddSlot()
	            .HAlign(HAlign_Fill)
	            .VAlign(VAlign_Fill)
	            .FillHeight(1.0f)
	[
		PinToAdd
	];
	OutputPins.Add(PinToAdd);
}

TSharedPtr<SToolTip> SHBGraphNodeConduit::GetComplexTooltip()
{
	UMontageGraphEdNodeConduit* StateNode = CastChecked<UMontageGraphEdNodeConduit>(GraphNode);

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
		// 	.CornerOverlayText(this, &SHBMontageGraphNodeConduit::GetPreviewCornerText)
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

FSlateColor SHBGraphNodeConduit::GetBorderBackgroundColor() const
{
	// Intentionally basic (might consider adding debug states here)
	constexpr FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
	FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
	FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);

	return InactiveStateColor;
}

const FSlateBrush* SHBGraphNodeConduit::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("Graph.ConduitNode.Icon"));
}

#undef LOCTEXT_NAMESPACE