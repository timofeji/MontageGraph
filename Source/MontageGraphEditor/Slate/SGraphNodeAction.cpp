#include "SGraphNodeAction.h"

#include "Slate/SMontageGraphNodeRow.h"
#include "SCommentBubble.h"
#include "SGraphPin.h"
#include "GraphEditorSettings.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "MontageGraphEditorStyles.h"
#include "Graph/MontageGraphDragDropAction.h"
#include "Graph/Nodes/MontageGraphEdNode.h"


#include "MontageGraph/MontageGraphNode.h"

#define LOCTEXT_NAMESPACE "SMontageGraphNode"

//////////////////////////////////////////////////////////////////////////
///
class SMontageGraphPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SMontageGraphPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin)
	{
		this->SetCursor(EMouseCursor::Default);

		bShowLabel = true;

		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);

		const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
		check(Schema);

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &SMontageGraphPin::GetPinBorder)
			.BorderBackgroundColor(this, &SMontageGraphPin::GetPinColor)
			.OnMouseButtonDown(this, &SMontageGraphPin::OnPinMouseDown)
			.Cursor(this, &SMontageGraphPin::GetPinCursor)
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			
		);
	}

protected:
	virtual FSlateColor GetPinColor() const override
	{
		constexpr FLinearColor Default(0.02f, 0.02f, 0.2f);
		// return Default;

		return FSlateColor(IsHovered() ? FLinearColor::Blue : Default);
	}

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}

	const FSlateBrush* GetPinBorder() const
	{
		FSlateBrush* Brush = const_cast<FSlateBrush*>(FMontageGraphEditorStyles::Get().GetBrush(TEXT("HBEditor.MontageGraph.Node.BackgroundLight")));
		Brush->DrawAs = ESlateBrushDrawType::Box;
		Brush->Margin = FMargin(0.5f);
		return Brush;
	}

	virtual TSharedRef<FDragDropOperation> SpawnPinDragEvent(const TSharedRef<class SGraphPanel>& InGraphPanel, const TArray<TSharedRef<SGraphPin>>& InStartingPins) override
	{
		TArray<FGraphPinHandle> PinHandles;
		PinHandles.Reserve(InStartingPins.Num());

		// Since the graph can be refreshed and pins can be reconstructed/replaced behind the scenes, the DragDropOperation holds onto FGraphPinHandles
		// instead of direct widgets/graph-pins
		for (const TSharedRef<SGraphPin>& PinWidget : InStartingPins)
		{
			PinHandles.Add(PinWidget->GetPinObj());
		}

		return FMontageGraphDragDropAction::New(InGraphPanel, PinHandles);
	}

};


//////////////////////////////////////////////////////////////////////////
void SGraphNodeAction::Construct(const FArguments& InArgs, UMontageGraphEdNode* InNode)
{
	check(InNode)
	GraphNode = InNode;
	CachedGraphNode = InNode;
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

void SGraphNodeAction::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (CachedGraphNode)
	{
		CachedGraphNode->UpdateTime(InDeltaTime);
	}
}

void SGraphNodeAction::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	constexpr FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f, 0.3f);
	const FSlateBrush* NodeTypeIcon = GetNameIcon();

	const TSharedPtr<SVerticalBox> NodeContent = CreateNodeContent();

	ContentScale.Bind( this, &SGraphNode::GetContentScale );
	GetOrAddSlot( ENodeZone::Center )
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor( this, &SGraphNodeAction::GetBorderBackgroundColor )
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
					.BorderImage( FAppStyle::GetBrush("Graph.StateNode.ColorSpill") )
					.BorderBackgroundColor( TitleShadowColor )
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Visibility(EVisibility::SelfHitTestInvisible)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							// POPUP ERROR MESSAGE
							SAssignNew(ErrorText, SErrorText )
							.BackgroundColor( this, &SGraphNodeAction::GetErrorColor )
							.ToolTipText( this, &SGraphNodeAction::GetErrorMsgToolTip )
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
								.AutoHeight()
							[
								SAssignNew(InlineEditableText, SInlineEditableTextBlock)
								.Style( FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText" )
								.Text( NodeTitle.Get(), &SNodeTitle::GetHeadTitle )
								.OnVerifyTextChanged(this, &SGraphNodeAction::OnVerifyNameTextChanged)
								.OnTextCommitted(this, &SGraphNodeAction::OnNameTextCommited)
								.IsReadOnly( this, &SGraphNodeAction::IsNameReadOnly )
								.IsSelected(this, &SGraphNodeAction::IsSelectedExclusively)
							]
							+SVerticalBox::Slot()
								.AutoHeight()
							[
								NodeTitle.ToSharedRef()
							]
						]
					]
				]
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

	ErrorReporting = ErrorText;
	ErrorColor = FAppStyle::GetColor("ErrorReporting.BackgroundColor");
	//ErrorReporting->SetError(ErrorMsg);

	CreatePinWidgets();
}

void SGraphNodeAction::CreatePinWidgets()
{
	UMontageGraphEdNode* StateNode = CastChecked<UMontageGraphEdNode>(GraphNode);
	
	UEdGraphPin* CurPin = StateNode->GetOutputPin();
	if (!CurPin->bHidden)
	{
		TSharedPtr<SGraphPin> NewPin = SNew(SMontageGraphPin, CurPin);

		AddPin(NewPin.ToSharedRef());
	}
}

void SGraphNodeAction::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	const FName PinName = PinObj ? PinObj->PinName : NAME_None;
	
	RightNodeBox->AddSlot()
    		.HAlign(HAlign_Fill)
    		.VAlign(VAlign_Fill)
    		.FillHeight(1.0f)
    		[
    			PinToAdd
    		];
	OutputPins.Add(PinToAdd);
	
}

bool SGraphNodeAction::IsNameReadOnly() const
{
	UMontageGraphEdNode* EdNode_Node = Cast<UMontageGraphEdNode>(GraphNode);
	check(EdNode_Node != nullptr);

	return (!EdNode_Node->RuntimeNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
}

void SGraphNodeAction::OnNameTextCommited(const FText& InText, const ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	UMontageGraphEdNode* MyNode = CastChecked<UMontageGraphEdNode>(GraphNode);

	if (MyNode != nullptr && MyNode->RuntimeNode != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("ACEGraphEditorRenameNode", "ACE Graph Editor: Rename Node"));
		MyNode->Modify();
		MyNode->RuntimeNode->Modify();
		MyNode->RuntimeNode->SetNodeTitle(InText);
		UpdateGraphNode();
	}
	
}

FSlateColor SGraphNodeAction::GetBorderBackgroundColor() const
{
	const UMontageGraphEdNode* MyNode = CastChecked<UMontageGraphEdNode>(GraphNode);
	return MyNode->GetBackgroundColor();
}

EVisibility SGraphNodeAction::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush* SGraphNodeAction::GetNameIcon() const
{
	return FMontageGraphEditorStyles::Get().GetBrush(TEXT("MontageGraph.Icon"));
}

void SGraphNodeAction::GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const
{
	if (!CachedGraphNode)
	{
		return;
	}

	const UMontageGraphNode* DebuggedNode = CachedGraphNode->GetDebuggedNode();
	if (!DebuggedNode)
	{
		return;
	}

	if (CachedGraphNode->IsDebugActive())
	{
		const FString Message = FString::Printf(TEXT("Active for %.2f secs"), CachedGraphNode->GetDebugTotalTime());
		new (Popups) FGraphInformationPopupInfo(nullptr, CachedGraphNode->GetBackgroundColor(), Message);
		
		const FString Message2 = FString::Printf(TEXT("Evaluated in %.5f secs"), CachedGraphNode->GetEvaluationTime());
		new (Popups) FGraphInformationPopupInfo(nullptr, CachedGraphNode->GetBackgroundColor().Desaturate(0.2f), Message2);
		
	}
	else if (CachedGraphNode->WasDebugActive())
	{
		const FString Message = FString::Printf(TEXT("Was Active for %.2f secs"), CachedGraphNode->GetDebugElapsedTime());
		new (Popups) FGraphInformationPopupInfo(nullptr, CachedGraphNode->GetBackgroundColor(), Message);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SGraphNodeAction::SetErrorText(const FText InErrorText)
{
	if (ErrorReporting)
	{
		ErrorReporting->SetError(InErrorText);
	}
}

FText SGraphNodeAction::GetMontageText()
{
	if (!CachedGraphNode || !CachedGraphNode->RuntimeNode)
	{
		return FText::FromString("");
	}

	return CachedGraphNode->RuntimeNode->GetNodeTitle();
}

TSharedPtr<SVerticalBox> SGraphNodeAction::CreateNodeContent()
{
	TSharedPtr<SVerticalBox> Content = SNew(SVerticalBox);
	const TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	Content->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	[
		SAssignNew(InlineEditableText, SInlineEditableTextBlock)
		.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
		.OnVerifyTextChanged(this, &SGraphNodeAction::OnVerifyNameTextChanged)
		.OnTextCommitted(this, &SGraphNodeAction::OnNameTextCommited)
		.IsReadOnly(this, &SGraphNodeAction::IsNameReadOnly)
		.IsSelected(this, &SGraphNodeAction::IsSelectedExclusively)
		.Justification(ETextJustify::Center)
	];

	Content->AddSlot()
	.AutoHeight()
	[
		NodeTitle.ToSharedRef()
	];

	if (!CachedGraphNode || !CachedGraphNode->RuntimeNode)
	{
		return Content;
	}

	const FMargin RowPadding = FMargin(0.f, 0.f, 0.f, 0.f);
	const FMargin TopRowPadding = FMargin(0.f, 0.f, 0.f, 0.f);
	const FMargin BottomRowPadding = FMargin(0.f, 0.f, 0.f, 0.f);

	Content->AddSlot()
	       .Padding(TopRowPadding)
	       .AutoHeight()
	[
		SNew(SMontageGraphNodeRow, CachedGraphNode->RuntimeNode)
		.IsTransparent(false)
		// .LabelText(CachedGraphNode->RuntimeNode->GetAnimAssetLabel())
		// .LabelTooltipText(CachedGraphNode->RuntimeNode->GetAnimAssetLabelTooltip())
		// .ValueTooltipText(CachedGraphNode->RuntimeNode->GetAnimAssetLabelTooltip())
		.OnGetValueText(FACEOnGetValueText::CreateSP(this, &SGraphNodeAction::GetMontageText))
	];

	/*
	Content->AddSlot()
	       .Padding(RowPadding)
	       .AutoHeight()
	[
		SNew(SMontageGraphNodeRow, CachedGraphNode->RuntimeNode)
		.IsTransparent(false)
		.LabelText(LOCTEXT("PlayRate", "Rate"))
		.LabelTooltipText(LOCTEXT("PlayRateTooltip", "Montage Play Rate"))
		.ValueTooltipText(LOCTEXT("PlayRateValueTooltip", "Montage Play Rate"))
		// .OnGetValueText(FACEOnGetValueText::CreateSP(this, &SHBMontageGraphNode::GetMontagePlayRateText))
	];

	if (CachedGraphNode->RuntimeNode)
	{
		Content->AddSlot()
		       .Padding(RowPadding)
		       .AutoHeight()
		[
			SNew(SMontageGraphNodeRow, CachedGraphNode->RuntimeNode)
			.IsTransparent(false)
			.LabelText(LOCTEXT("StartSection", "Montage Start Section"))
			.LabelTooltipText(LOCTEXT("StartSection", "Montage Start Section"))
			.ValueTooltipText(LOCTEXT("StartSection", "Montage Start Section"))
			.OnGetValueText(FACEOnGetValueText::CreateSP(this, &SHBMontageGraphNode::GetMontageStartSectionName))
			.Visibility(this, &SHBMontageGraphNode::GetMontageStartSectionNameVisibility)
		];
	}

	Content->AddSlot()
	       .Padding(RowPadding)
	       .AutoHeight()
	[
		SNew(SMontageGraphNodeRow, CachedGraphNode->RuntimeNode)
		.IsTransparent(false)
		.LabelText(LOCTEXT("EffectCost", "Effect Cost"))
		.LabelTooltipText(LOCTEXT("EffectCost", "Effect Cost"))
		.ValueTooltipText(LOCTEXT("EffectCost", "Effect Cost"))
		// .OnGetValueText(FACEOnGetValueText::CreateSP(this, &SHBMontageGraphNode::GetGameplayEffectCostText))
		// .Visibility(this, &SHBMontageGraphNode::GetGameplayEffectCostTextVisibility)
	];*/

	if (CachedGraphNode->RuntimeNode->IsHBActionSubclassedInBlueprint())
	{
		FString Classname = GetNameSafe(CachedGraphNode->RuntimeNode->GetClass());
		Classname.RemoveFromEnd("_C");

		Content->AddSlot()
		       .Padding(BottomRowPadding)
		       .AutoHeight()
		[
			SNew(SMontageGraphNodeRow, CachedGraphNode->RuntimeNode)
			.IsTransparent(true)
			// .LabelText(LOCTEXT("NodeClass", "Node Class"))
			.LabelTooltipText(LOCTEXT("NodeClass", "Node Class"))
			.ValueTooltipText(LOCTEXT("NodeClass", "Node Class"))
			.OnGetValueText(FACEOnGetValueText::CreateLambda([Classname]()
			{
				return FText::FromString(*Classname);
			}))
		];
	}

	return Content;
}

//
// EVisibility SHBMontageGraphNode::GetGameplayEffectCostTextVisibility() const
// {
// 	const TSubclassOf<UGameplayEffect> CostGE = CachedGraphNode->RuntimeNode->CostGameplayEffect;
// 	return IsValid(CostGE) ? EVisibility::Visible : EVisibility::Collapsed;
// }
//
// FText SHBMontageGraphNode::GetGameplayEffectCostText() const
// {
// 	const TSubclassOf<UGameplayEffect> CostGE = CachedGraphNode->RuntimeNode->CostGameplayEffect;
// 	return FText::FromString(CostGE ? CostGE->GetName() : TEXT("NONE"));
// }


FText SGraphNodeAction::GetMontageStartSectionName() const
{
	FName StartSectionName = NAME_None;
	return FText::FromName(StartSectionName);
}

EVisibility SGraphNodeAction::GetMontageStartSectionNameVisibility() const
{
	if (!CachedGraphNode || !CachedGraphNode->RuntimeNode)
	{
		return EVisibility::Collapsed;
	}

	const FName StartSectionName ;
	return StartSectionName != NAME_None ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SGraphNodeAction::GetErrorText() const
{
	return LOCTEXT("ErrorText", "Test Error Yo");
}


#undef LOCTEXT_NAMESPACE
