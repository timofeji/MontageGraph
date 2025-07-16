// Created by Timofej Jermolaev, All rights reserved . 


#include "SMGNode_Entry.h"

#include "MontageGraphEditorStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Slate/SMontageGraphEntryPin.h"
#include "Graph\EdNodes\MGEdNode_Entry.h"

#define LOCTEXT_NAMESPACE "SMGNode_Entry"
/////////////////////////////////////////////////////
// SMGNode_Entry

void SMGNode_Entry::Construct(const FArguments& InArgs, UMGEdNode_Entry* InNode)
{
	GraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);

	UpdateGraphNode();
}

void SMGNode_Entry::GetNodeInfoPopups(FNodeInfoContext* Context,
                                                TArray<FGraphInformationPopupInfo>& Popups) const
{
}

const bool SMGNode_Entry::IsTitleVisible() const
{
	return false;
}

FSlateColor SMGNode_Entry::GetBorderBackgroundColor() const
{
	constexpr FLinearColor InactiveStateColor(1.f, 1.f, 1.f, 0.07f);
	return InactiveStateColor;
}

// void SMGNode_Entry::UpdateGraphNode()
// {
// 	InputPins.Empty();
// 	OutputPins.Empty();
//
// 	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
// 	RightNodeBox.Reset();
//
// 	//
// 	//             ______________________
// 	//            |      TITLE AREA      |
// 	//            +-------+------+-------+
// 	//            | (>) L |      | R (>) |
// 	//            | (>) E |      | I (>) |
// 	//            | (>) F |      | G (>) |
// 	//            | (>) T |      | H (>) |
// 	//            |       |      | T (>) |
// 	//            |_______|______|_______|
// 	//
//
// 	TSharedPtr<SErrorText> ErrorText;
// 	constexpr FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f, 0.3f);
//
// 	// const TSharedPtr<SVerticalBox> NodeContent = CreateNodeContent();
//
// 	ContentScale.Bind(this, &SGraphNode::GetContentScale);
// 	GetOrAddSlot(ENodeZone::Center)
// 		.HAlign(HAlign_Fill)
// 		.VAlign(VAlign_Fill)
// 		[
// 			SNew(SOverlay)
// 			+ SOverlay::Slot()
// 			  .HAlign(HAlign_Fill)
// 			  .VAlign(VAlign_Fill)
// 			[
// 				SNew(SBorder)
//                     			// .BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
//                     			.BorderImage(FMontageGraphEditorStyle::Get().GetBrush("HBEditor.MontageGraph.Entry"))
//                     			.Padding(10.f)
//                     			.DesiredSizeScale(FVector2d(1.f, 1.f))
//                     			.BorderBackgroundColor(this, &SMGNode_Entry::GetBorderBackgroundColor)
// 				[
// 					SAssignNew(RightNodeBox, SVerticalBox)
// 				]
// 			]
//
// 		];
//
// 	// // Create comment bubble
// 	// TSharedPtr<SCommentBubble> CommentBubble;
// 	// const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;
// 	//
// 	// SAssignNew(CommentBubble, SCommentBubble)
// 	//    		.GraphNode(GraphNode)
// 	//    		.Text(this, &SGraphNode::GetNodeComment)
// 	//    		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
// 	//    		.ColorAndOpacity(CommentColor)
// 	//    		.AllowPinning(true)
// 	//    		.EnableTitleBarBubble(true)
// 	//    		.EnableBubbleCtrls(true)
// 	//    		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
// 	//    		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);
// 	//
// 	// GetOrAddSlot(ENodeZone::TopCenter)
// 	// 	.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
// 	// 	.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
// 	// 	.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
// 	// 	.VAlign(VAlign_Top)
// 	// 	[
// 	// 		CommentBubble.ToSharedRef()
// 	// 	];
//
// 	// ErrorReporting = ErrorText;
// 	// ErrorColor = FAppStyle::GetColor("ErrorReporting.BackgroundColor");
// 	// ErrorReporting->SetError(ErrorMsg);
//
// 	CreatePinWidgets();
// }
//
// void SMGNode_Entry::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
// {
// 	// PinToAdd->SetOwner(SharedThis(this));
// 	// RightNodeBox->AddSlot()
// 	//             .HAlign(HAlign_Fill)
// 	//             .VAlign(VAlign_Fill)
// 	//             .FillHeight(1.0f)
// 	// [
// 	// 	PinToAdd
// 	// ];
// 	// OutputPins.Add(PinToAdd);
// }
//
// void SMGNode_Entry::CreatePinWidgets()
// {
// 	// for (UEdGraphPin* Pin : GraphNode->Pins)
// 	// {
// 	// 	if (Pin->Direction == EGPD_Output)
// 	// 	{
// 	// 		TSharedPtr<SGraphPin> NewPin = SNew(SMontageGraphEntryPin, Pin);
// 	// 		AddPin(NewPin.ToSharedRef());
// 	// 	}
// 	// }
//
// 	// CurPin = GraphNode->GetInputPin();
// 	// if (CurPin && !CurPin->bHidden)
// 	// {
// 	// 	TSharedPtr<SGraphPin> NewPin = SNew(SEnvironmentQueryPin, CurPin);
// 	//
// 	// 	AddPin(NewPin.ToSharedRef());
// 	// }
// 	//
// 	// Super::CreatePinWidgets();
// }

FText SMGNode_Entry::GetPreviewCornerText() const
{
	return NSLOCTEXT("SMGNode_Entry", "CornerTextDescription", "Entry point for state machine");
}
#undef LOCTEXT_NAMESPACE
