// Copyright Drop Games Inc.
#include "SMGNode.h"
#include "SCommentBubble.h"
#include "SGraphPin.h"
#include "GraphEditorSettings.h"
#include "IDocumentation.h"
#include "MontageGraphEditorStyle.h"
#include "SGraphPanel.h"
#include "Graph/EdNodes/MGEdNode.h"
#include "MontageGraph/Nodes/MGNode.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SMGNode"

//////////////////////////////////////////////////////////////////////////
void SMGNode::Construct(const FArguments& InArgs, UMGEdNode* InNode)
{
	check(InNode)
	GraphNode = InNode;
	ContentPadding = InArgs._ContentPadding;
	ContentInternalPadding = InArgs._ContentInternalPadding;
	PinSize = InArgs._PinSize;
	PinPadding = InArgs._PinPadding;
	bDrawVerticalPins = InArgs._DrawVerticalPins;
	bDrawHorizontalPins = InArgs._DrawHorizontalPins;
	UpdateGraphNode();
	SetCursor(EMouseCursor::CardinalCross);
	InNode->SlateNode = this;
}

void SMGNode::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (UMGEdNode* MGGraphEdNode = Cast<UMGEdNode>(GraphNode))
	{
		MGGraphEdNode->UpdateTime(InDeltaTime);
	}
}

//
void SMGNode::UpdateGraphNode()
{
		InputPins.Empty();
    	OutputPins.Empty();
    	
    	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
    	RightNodeBox.Reset();
    	LeftNodeBox.Reset();
    
    	const FSlateBrush* NodeTypeIcon = GetNameIcon();
    
    	FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
    	TSharedPtr<SErrorText> ErrorText;
    	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
    
    	this->ContentScale.Bind( this, &SGraphNode::GetContentScale );
    	this->GetOrAddSlot( ENodeZone::Center )
    		.HAlign(HAlign_Center)
    		.VAlign(VAlign_Center)
    		[
    			SNew(SBorder)
			    .BorderImage(GetNodeBodyBrush())
    			.Padding(0)
			    .BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
    			[
    				SNew(SOverlay)
    
    				// PIN AREA
    				+SOverlay::Slot()
    				.HAlign(HAlign_Fill)
    				.VAlign(VAlign_Fill)
    				[
    					SAssignNew(RightNodeBox, SVerticalBox)
    				]
    
    				// STATE NAME AREA
    				+SOverlay::Slot()
    				.HAlign(HAlign_Center)
    				.VAlign(VAlign_Center)
    				.Padding(10.0f)
    				[
    					SNew(SBorder)
    					.BorderImage( FMontageGraphEditorStyle::Get().GetBrush("Graph.Node.ColorSpill") )
    					.BorderBackgroundColor( TitleShadowColor )
    					.HAlign(HAlign_Center)
    					.VAlign(VAlign_Center)
    					.Visibility(EVisibility::SelfHitTestInvisible)
					    .Padding(5.0f)
    					[
    						SNew(SHorizontalBox)
    						+SHorizontalBox::Slot()
    						.AutoWidth()
    						[
    							// POPUP ERROR MESSAGE
    							SAssignNew(ErrorText, SErrorText )
    							// .BackgroundColor( this, &SGraphNodeAnimState::GetErrorColor )
    							// .ToolTipText( this, &SGraphNodeAnimState::GetErrorMsgToolTip )
    						]
    						+SHorizontalBox::Slot()
    						.AutoWidth()
    						.VAlign(VAlign_Center)
    						[
    							SNew(SImage)
    							.Image(NodeTypeIcon)
    						]
    						+SHorizontalBox::Slot()
    						.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
    						[
    							SNew(SVerticalBox)
    							+SVerticalBox::Slot()
							    .VAlign(VAlign_Center)
							    .HAlign(HAlign_Center)
							    .AutoHeight()
    							[
    								
								CreateTitleWidget(NodeTitle)
							    ]
							    + SVerticalBox::Slot()
							    .AutoHeight()
							    [

								    NodeTitle.ToSharedRef()
							    ]
						    ]
					    ]
				    ]
			    ]
		    ];
    
    	ErrorReporting = ErrorText;
    	ErrorReporting->SetError(ErrorMsg);
    	CreatePinWidgets();
	
	
	
	
	
	
	//
	// InputPins.Empty();
	// OutputPins.Empty();
	//
	// // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	// RightNodeBox.Reset();
	// LeftNodeBox.Reset();
	//
	// //
	// //             ______________________
	// //            |      TITLE AREA      |
	// //            +-------+------+-------+
	// //            | (>) L |      | R (>) |
	// //            | (>) E |      | I (>) |
	// //            | (>) F |      | G (>) |
	// //            | (>) T |      | H (>) |
	// //            |       |      | T (>) |
	// //            |_______|______|_______|
	// //
	// TSharedPtr<SVerticalBox> MainVerticalBox;
	// SetupErrorReporting();
	//
	// TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	//
	// // Get node icon
	// IconColor = FLinearColor::White;
	// const FSlateBrush* IconBrush = nullptr;
	// if (GraphNode != NULL && GraphNode->ShowPaletteIconOnNode())
	// {
	// 	IconBrush = GraphNode->GetIconAndTint(IconColor).GetOptionalIcon();
	// }
	//
	// TSharedRef<SOverlay> DefaultTitleAreaWidget =
	// 	SNew(SOverlay)
	//
	// 	+ SOverlay::Slot()
	// 	.HAlign(HAlign_Fill)
	// 	.VAlign(VAlign_Center)
	// 	[
	// 		SNew(SHorizontalBox)
	// 		+ SHorizontalBox::Slot()
	// 		.HAlign(HAlign_Fill)
	// 		[
	// 			SNew(SBorder)
	// 			             .BorderImage(FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.Node.Title.Overlay"))
	// 						.Padding(TitleBorderMargin)
	// 			             // .Padding(FMargin(1.f, 1.f, 1.f, 0.f))
	// 			             .BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
	// 			[
	// 				SNew(SBorder)
	// 				.BorderImage(FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.Node.Title.Overlay.Body"))
	// 				.Padding(FMargin(TitleBorderMargin))
	//
	// 				.HAlign(HAlign_Fill)
	// 				.VAlign(VAlign_Fill)
	// 				.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
	// 				[
	//
	// 					SNew(SHorizontalBox)
	// 					+ SHorizontalBox::Slot()
	// 					  .VAlign(VAlign_Center)
	// 					  .Padding(FMargin(0.f))
	// 					  // .Padding(FMargin(1.f, 1.f, 1.f, 0.f))
	// 					  .AutoWidth()
	// 					[
	// 						SNew(SImage)
	// 						.Image(IconBrush)
	// 						.ColorAndOpacity(this, &SGraphNode::GetNodeTitleIconColor)
	// 					]
	// 					+ SHorizontalBox::Slot()
	// 					.VAlign(VAlign_Center)
	// 					.HAlign(HAlign_Center)
	// 					[
	// 						SNew(SVerticalBox)
	// 						+ SVerticalBox::Slot()
	// 						.VAlign(VAlign_Center)
	// 						.HAlign(HAlign_Center)
	// 						.AutoHeight()
	// 						[
	// 							CreateTitleWidget(NodeTitle)
	// 						]
	// 						+ SVerticalBox::Slot()
	// 						.VAlign(VAlign_Center)
	// 						.HAlign(HAlign_Center)
	// 						.AutoHeight()
	// 						[
	// 							NodeTitle.ToSharedRef()
	// 						]
	// 					]
	// 				]
	// 			]
	// 		]
	// 		+ SHorizontalBox::Slot()
	// 		.HAlign(HAlign_Right)
	// 		.VAlign(VAlign_Center)
	// 		.Padding(0, 0, 0, 0)
	// 		.AutoWidth()
	// 		[
	// 			CreateTitleRightWidget()
	// 		]
	//
	// 	];
	//
	// if (IsTitleVisible())
	// {
	// 	SetDefaultTitleAreaWidget(DefaultTitleAreaWidget);
	// }
	//
	// const TSharedRef<SWidget> TitleAreaWidget =
	// 	SNew(SLevelOfDetailBranchNode)
	// 	.Visibility_Lambda([this]()
	// 	{
	// 		return IsTitleVisible() ? EVisibility::Visible : EVisibility::Collapsed;
	// 	})
	// 	.UseLowDetailSlot(this, &SMGNode::UseLowDetailNodeTitles)
	// 	.LowDetail()
	// 	[
	// 		SNew(SBorder)
	// 		.BorderImage(FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.Node.Title"))
	// 		.Padding(FMargin(75.0f, 22.0f))
	// 		.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
	// 	]
	// 	.HighDetail()
	// 	[
	// 		DefaultTitleAreaWidget
	// 	];
	//
	//
	// if (!SWidget::GetToolTip().IsValid())
	// {
	// 	TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip(
	// 		TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(),
	// 		GraphNode->GetDocumentationExcerptName());
	// 	SetToolTip(DefaultToolTip);
	// }
	//
	// // Setup a meta tag for this node
	// FGraphNodeMetaData TagMeta(TEXT("Graphnode"));
	// PopulateMetaTag(&TagMeta);
	//
	// TSharedPtr<SVerticalBox> InnerVerticalBox;
	// this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	//
	//
	// InnerVerticalBox = SNew(SVerticalBox)
	// 	+ SVerticalBox::Slot()
	// 	  .AutoHeight()
	// 	  .HAlign(HAlign_Fill)
	// 	  .VAlign(VAlign_Top)
	// 	  .Padding(Settings->GetNonPinNodeBodyPadding())
	// 	  .Padding(FMargin(0.f))
	// 	[
	// 		TitleAreaWidget
	// 	]
	//
	// 	+ SVerticalBox::Slot()
	// 	.AutoHeight()
	// 	.HAlign(HAlign_Fill)
	// 	.VAlign(VAlign_Top)
	// 	.Padding(FMargin(0.f))
	// 	[
	// 		CreateNodeContentArea()
	// 	];
	//
	// TSharedPtr<SWidget> EnabledStateWidget = GetEnabledStateWidget();
	// if (EnabledStateWidget.IsValid())
	// {
	// 	InnerVerticalBox->AddSlot()
	// 	                .AutoHeight()
	// 	                .HAlign(HAlign_Fill)
	// 	                .VAlign(VAlign_Top)
	// 	                .Padding(FMargin(2, 0))
	// 	[
	// 		EnabledStateWidget.ToSharedRef()
	// 	];
	// }
	//
	// InnerVerticalBox->AddSlot()
	//                 .AutoHeight()
	//                 .Padding(Settings->GetNonPinNodeBodyPadding())
	// [
	// 	ErrorReporting->AsWidget()
	// ];
	//
	// InnerVerticalBox->AddSlot()
	//                 .AutoHeight()
	//                 .Padding(Settings->GetNonPinNodeBodyPadding())
	// [
	// 	VisualWarningReporting->AsWidget()
	// ];
	//
	//
	// this->GetOrAddSlot(ENodeZone::Center)
	//     .HAlign(HAlign_Center)
	//     .VAlign(VAlign_Center)
	// [
	// 	SAssignNew(MainVerticalBox, SVerticalBox)
	// 	+ SVerticalBox::Slot()
	// 	.AutoHeight()
	// 	[
	// 		SNew(SOverlay)
	// 		.AddMetaData<FGraphNodeMetaData>(TagMeta)
	// 		+ SOverlay::Slot()
	// 		.Padding(Settings->GetNonPinNodeBodyPadding())
	// 		[
	// 			SNew(SImage)
	// 			.Image(GetNodeBodyBrush())
	// 			.ColorAndOpacity(this, &SGraphNode::GetNodeTitleColor)
	// 		]
	// 		+ SOverlay::Slot()
	// 		[
	// 			InnerVerticalBox.ToSharedRef()
	// 		]
	// 	]
	// ];
	//
	// bool SupportsBubble = true;
	// if (GraphNode != nullptr)
	// {
	// 	SupportsBubble = GraphNode->SupportsCommentBubble();
	// }
	//
	// if (SupportsBubble)
	// {
	// 	// Create comment bubble
	// 	TSharedPtr<SCommentBubble> CommentBubble;
	// 	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;
	//
	// 	SAssignNew(CommentBubble, SCommentBubble)
	// 	.GraphNode(GraphNode)
	// 	.Text(this, &SGraphNode::GetNodeComment)
	// 	.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
	// 	.OnToggled(this, &SGraphNode::OnCommentBubbleToggled)
	// 	.ColorAndOpacity(CommentColor)
	// 	.AllowPinning(true)
	// 	.EnableTitleBarBubble(true)
	// 	.EnableBubbleCtrls(true)
	// 	.GraphLOD(this, &SGraphNode::GetCurrentLOD)
	// 	.IsGraphNodeHovered(this, &SGraphNode::IsHovered);
	//
	// 	GetOrAddSlot(ENodeZone::TopCenter)
	// 		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
	// 		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
	// 		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
	// 		.VAlign(VAlign_Top)
	// 		[
	// 			CommentBubble.ToSharedRef()
	// 		];
	// }
	//
	// CreateBelowWidgetControls(MainVerticalBox);
	// CreatePinWidgets();
	// CreateInputSideAddButton(LeftNodeBox);
	// CreateOutputSideAddButton(RightNodeBox);
	// CreateBelowPinControls(InnerVerticalBox);
	// CreateAdvancedViewArrow(InnerVerticalBox);
}


void SMGNode ::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Add pins to the hover set so outgoing transitions arrows remains highlighted while the mouse is over the state node
	if (const UMGEdNode* MGEdNode = Cast<UMGEdNode>(GraphNode))
	{
		if (const UEdGraphPin* OutputPin = MGEdNode->GetOutputPin())
		{
			TSharedPtr<SGraphPanel> OwnerPanel = GetOwnerPanel();
			check(OwnerPanel.IsValid());

			for (int32 LinkIndex = 0; LinkIndex < OutputPin->LinkedTo.Num(); ++LinkIndex)
			{
				OwnerPanel->AddPinToHoverSet(OutputPin->LinkedTo[LinkIndex]);
			}
		}
	}
	
	SGraphNode::OnMouseEnter(MyGeometry, MouseEvent);
}

void SMGNode ::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	// Remove manually added pins from the hover set
	if (const UMGEdNode* MGEdNode = Cast<UMGEdNode>(GraphNode))
	{
		if(const UEdGraphPin* OutputPin = MGEdNode->GetOutputPin())
		{
			TSharedPtr<SGraphPanel> OwnerPanel = GetOwnerPanel();
			check(OwnerPanel.IsValid());

			for (int32 LinkIndex = 0; LinkIndex < OutputPin->LinkedTo.Num(); ++LinkIndex)
			{
				OwnerPanel->RemovePinFromHoverSet(OutputPin->LinkedTo[LinkIndex]);
			}
		}
	}

	SGraphNode::OnMouseLeave(MouseEvent);
}



class SMGNodePin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SMGNodePin ){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
	// Begin SGraphPin interface
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
	virtual FSlateColor        GetPinColor() const override;
	// End SGraphPin interface

	const FSlateBrush* GetPinBorder() const;
};

void SMGNodePin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	this->SetCursor( EMouseCursor::Default );

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	// Set up a hover for pins that is tinted the color of the pin.
	SBorder::Construct( SBorder::FArguments()
		.BorderImage( this, &SMGNodePin::GetPinBorder )
		.BorderBackgroundColor( this, &SMGNodePin::GetPinColor )
		.OnMouseButtonDown( this, &SMGNodePin::OnPinMouseDown )
		.Cursor( this, &SMGNodePin::GetPinCursor )
	);
}
TSharedRef<SWidget>	SMGNodePin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SMGNodePin::GetPinBorder() const
{
	return ( IsHovered() )
		? FMontageGraphEditorStyle::Get().GetBrush( TEXT("Graph.Node.Pin.BackgroundHovered") )
		: FMontageGraphEditorStyle::Get().GetBrush( TEXT("Graph.Node.Pin.Background") );
}

FSlateColor SMGNodePin::GetPinColor() const
{
	UEdGraphPin* GraphPin = GetPinObj();
	if (GraphPin && !GraphPin->IsPendingKill())
	{
		if (bIsDiffHighlighted)
		{
			return FSlateColor(FLinearColor(0.9f, 0.2f, 0.15f));
		}
		if (GraphPin->bOrphanedPin)
		{
			return FSlateColor(FLinearColor::Red);
		}
		if (const UEdGraphSchema* Schema = GraphPin->GetSchema())
		{
			if (!GetPinObj()->GetOwningNode()->IsNodeEnabled() || GetPinObj()->GetOwningNode()->IsDisplayAsDisabledForced() || !IsEditingEnabled() || GetPinObj()->GetOwningNode()->IsNodeUnrelated())
			{
				return Schema->GetPinTypeColor(GraphPin->PinType) * FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);
			}

			return Schema->GetPinTypeColor(GraphPin->PinType) * PinColorModifier;
		}
	}

	return FLinearColor::White;
}

void SMGNode::CreatePinWidgets()
{
	UMGEdNode* EdNode = CastChecked<UMGEdNode>(GraphNode);

	UEdGraphPin* CurPin = EdNode ->GetOutputPin();
	if (CurPin && !CurPin->bHidden)
	{
		TSharedPtr<SGraphPin> NewPin = SNew(SMGNodePin , CurPin);

		this->AddPin(NewPin.ToSharedRef());
	}
}

void SMGNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

bool SMGNode::IsNameReadOnly() const
{
	UMGEdNode* EdNode_Node = Cast<UMGEdNode>(GraphNode);
	check(EdNode_Node != nullptr);

	return (!EdNode_Node->RuntimeNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
}

const FSlateBrush* SMGNode::GetShadowBrush(bool bSelected) const
{
	return bSelected
		       ? FMontageGraphEditorStyle::Get().GetBrush(TEXT("MontageGraph.Node.ActiveShadow"))
		       : FMontageGraphEditorStyle::Get().GetBrush(TEXT("MontageGraph.Node.Shadow"));
}

const FSlateBrush* SMGNode::GetNodeBodyBrush() const
{
	return FMontageGraphEditorStyle::Get().GetBrush(TEXT("MontageGraph.Node.Body"));
}

FSlateColor SMGNode::GetNodeTitleColor() const
{
	if(const UMGEdNode* MyNode = CastChecked<UMGEdNode>(GraphNode))
	{
		return MyNode->GetBackgroundColor();
	}
	
	FLinearColor ReturnTitleColor = GraphNode->IsDeprecated() ? FLinearColor::Red : GetNodeObj()->GetNodeTitleColor();
	return ReturnTitleColor;
}



EVisibility SMGNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush* SMGNode::GetNameIcon() const
{
	if (UMGEdNode* MGGraphEdNode = Cast<UMGEdNode>(GraphNode))
	{
		FLinearColor OutColor;
		return MGGraphEdNode->GetIconAndTint(OutColor).GetIcon();
	}
	
	return FMontageGraphEditorStyle::Get().GetBrush(TEXT("MontageGraph.Icon"));
}

const bool SMGNode::IsTitleVisible() const
{
	return true;
}

void SMGNode::GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const
{
	UMGEdNode* MGGraphEdNode = Cast<UMGEdNode>(GraphNode);
	if (!MGGraphEdNode)
	{
		return;
	}

	const UMGNode* DebuggedNode = MGGraphEdNode->RuntimeNode;
	if (!DebuggedNode)
	{
		return;
	}


	if (MGGraphEdNode->IsDebugActive())
	{
		const FString Message = FString::Printf(TEXT("Active for %.2f secs"), MGGraphEdNode->GetDebugTotalTime());
		new(Popups) FGraphInformationPopupInfo(nullptr, MGGraphEdNode->GetNodeTitleColor(), Message);

		const FString Message2 = FString::Printf(TEXT("Evaluated in %.5f secs"), MGGraphEdNode->GetEvaluationTime());
		new(Popups) FGraphInformationPopupInfo(nullptr, MGGraphEdNode->GetNodeTitleColor().Desaturate(0.5f),
		                                       Message2);
	}
	else if (MGGraphEdNode->WasDebugActive())
	{
		const FString Message = FString::Printf(
			TEXT("Was Active for %.2f secs"), MGGraphEdNode->GetDebugNormalizedTime());
		new(Popups) FGraphInformationPopupInfo(nullptr, MGGraphEdNode->GetBackgroundColor(), Message);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SMGNode::SetErrorText(const FText InErrorText)
{
	if (ErrorReporting)
	{
		ErrorReporting->SetError(InErrorText);
	}
}


bool SMGNode::UseLowDetailNode() const
{
	if (InlineEditableText.IsValid())
	{
		if (const SGraphPanel* MyOwnerPanel = GetOwnerPanel().Get())
		{
			return (MyOwnerPanel->GetCurrentLOD() <= EGraphRenderingLOD::LowestDetail) && !InlineEditableText->
				IsInEditMode();
		}
	}

	return false;
}

FText SMGNode::GetErrorText() const
{
	return LOCTEXT("ErrorText", "Test Error Yo");
}


#undef LOCTEXT_NAMESPACE
