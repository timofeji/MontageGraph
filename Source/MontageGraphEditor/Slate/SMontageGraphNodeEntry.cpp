// Created by Timofej Jermolaev, All rights reserved . 


#include "SMontageGraphNodeEntry.h"

#include "MontageGraphEditorStyles.h"
#include "Widgets/SBoxPanel.h"
#include "SMontageGraphEntryPin.h"
#include "..\Graph\Nodes\MontageGraphEdNodeEntry.h"

#define LOCTEXT_NAMESPACE "SMontageGraphNodeEntry"
/////////////////////////////////////////////////////
// SMontageGraphNodeEntry

void SMontageGraphNodeEntry::Construct(const FArguments& InArgs, UMontageGraphEdNodeEntry* InNode)
{
	GraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

void SMontageGraphNodeEntry::GetNodeInfoPopups(FNodeInfoContext* Context,
                                                TArray<FGraphInformationPopupInfo>& Popups) const
{
}

FSlateColor SMontageGraphNodeEntry::GetBorderBackgroundColor() const
{
	constexpr FLinearColor InactiveStateColor(1.f, 1.f, 1.f, 0.07f);
	return InactiveStateColor;
}

void SMontageGraphNodeEntry::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();

	//
	//             ______________________
	//            |      TITLE AREA      |
	//            +-------+------+-------+
	//            | (>) L |      | R (>) |
	//            | (>) E |      | I (>) |
	//            | (>) F |      | G (>) |
	//            | (>) T |      | H (>) |
	//            |       |      | T (>) |
	//            |_______|______|_______|
	//

	TSharedPtr<SErrorText> ErrorText;
	constexpr FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f, 0.3f);

	// const TSharedPtr<SVerticalBox> NodeContent = CreateNodeContent();

	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			  .HAlign(HAlign_Fill)
			  .VAlign(VAlign_Fill)
			[
				SNew(SBorder)
                    			// .BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
                    			.BorderImage(FMontageGraphEditorStyles::Get().GetBrush("HBEditor.MontageGraph.Entry"))
                    			.Padding(10.f)
                    			.DesiredSizeScale(FVector2d(1.f, 1.f))
                    			.BorderBackgroundColor(this, &SMontageGraphNodeEntry::GetBorderBackgroundColor)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
			]

		];

	// // Create comment bubble
	// TSharedPtr<SCommentBubble> CommentBubble;
	// const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;
	//
	// SAssignNew(CommentBubble, SCommentBubble)
	//    		.GraphNode(GraphNode)
	//    		.Text(this, &SGraphNode::GetNodeComment)
	//    		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
	//    		.ColorAndOpacity(CommentColor)
	//    		.AllowPinning(true)
	//    		.EnableTitleBarBubble(true)
	//    		.EnableBubbleCtrls(true)
	//    		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
	//    		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);
	//
	// GetOrAddSlot(ENodeZone::TopCenter)
	// 	.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
	// 	.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
	// 	.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
	// 	.VAlign(VAlign_Top)
	// 	[
	// 		CommentBubble.ToSharedRef()
	// 	];

	// ErrorReporting = ErrorText;
	// ErrorColor = FAppStyle::GetColor("ErrorReporting.BackgroundColor");
	// ErrorReporting->SetError(ErrorMsg);

	CreatePinWidgets();
}

void SMontageGraphNodeEntry::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

void SMontageGraphNodeEntry::CreatePinWidgets()
{
	for (UEdGraphPin* Pin : GraphNode->Pins)
	{
		if (Pin->Direction == EGPD_Output)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SMontageGraphEntryPin, Pin);
			AddPin(NewPin.ToSharedRef());
		}
	}

	// CurPin = GraphNode->GetInputPin();
	// if (CurPin && !CurPin->bHidden)
	// {
	// 	TSharedPtr<SGraphPin> NewPin = SNew(SEnvironmentQueryPin, CurPin);
	//
	// 	AddPin(NewPin.ToSharedRef());
	// }
	//
	// Super::CreatePinWidgets();
}

FText SMontageGraphNodeEntry::GetPreviewCornerText() const
{
	return NSLOCTEXT("SMontageGraphNodeEntry", "CornerTextDescription", "Entry point for state machine");
}
#undef LOCTEXT_NAMESPACE
