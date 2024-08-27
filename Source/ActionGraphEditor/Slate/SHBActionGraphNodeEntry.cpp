// Created by Timofej Jermolaev, All rights reserved . 


#include "SHBActionGraphNodeEntry.h"

#include "ActionGraphEditorStyles.h"
#include "Widgets/SBoxPanel.h"
#include "SHBActionGraphEntryPin.h"
#include "Graph/Nodes/HBActionGraphEdNodeEntry.h"

#define LOCTEXT_NAMESPACE "SHBActionGraphNodeEntry"
/////////////////////////////////////////////////////
// SHBActionGraphNodeEntry

void SHBActionGraphNodeEntry::Construct(const FArguments& InArgs, UActionGraphEdNodeEntry* InNode)
{
	GraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

void SHBActionGraphNodeEntry::GetNodeInfoPopups(FNodeInfoContext* Context,
                                                TArray<FGraphInformationPopupInfo>& Popups) const
{
}

FSlateColor SHBActionGraphNodeEntry::GetBorderBackgroundColor() const
{
	constexpr FLinearColor InactiveStateColor(1.f, 1.f, 1.f, 0.67f);
	return InactiveStateColor;
}

void SHBActionGraphNodeEntry::UpdateGraphNode()
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
                    			.BorderImage(FActionGraphEditorStyles::Get().GetBrush("HBEditor.ActionGraph.Entry"))
                    			.Padding(10.f)
                    			.DesiredSizeScale(FVector2d(1.f, 1.f))
                    			.BorderBackgroundColor(this, &SHBActionGraphNodeEntry::GetBorderBackgroundColor)
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

void SHBActionGraphNodeEntry::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

void SHBActionGraphNodeEntry::CreatePinWidgets()
{
	for (UEdGraphPin* Pin : GraphNode->Pins)
	{
		if (Pin->Direction == EGPD_Output)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SHBActionGraphEntryPin, Pin);
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

FText SHBActionGraphNodeEntry::GetPreviewCornerText() const
{
	return NSLOCTEXT("SHBActionGraphNodeEntry", "CornerTextDescription", "Entry point for state machine");
}
#undef LOCTEXT_NAMESPACE
