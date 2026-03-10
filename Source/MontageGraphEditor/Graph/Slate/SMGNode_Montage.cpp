#include "SMGNode_Montage.h"

#include "SGraphPin.h"
#include "Graph/EdNodes/MGEdNode.h"


#include "MontageGraph/Nodes/MGNode.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#define LOCTEXT_NAMESPACE "SMGNode"
//
// //////////////////////////////////////////////////////////////////////////
void SMGNode_Montage::Construct(const FArguments& InArgs, UMGEdNode* InNode)
{
	check(InNode)
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
	// ContentPadding = InArgs._ContentPadding;
	// ContentInternalPadding = InArgs._ContentInternalPadding;
	// PinSize = InArgs._PinSize;
	// PinPadding = InArgs._PinPadding;
	// bDrawVerticalPins = InArgs._DrawVerticalPins;
	// bDrawHorizontalPins = InArgs._DrawHorizontalPins;
	// SetCursor(EMouseCursor::CardinalCross);
	// InNode->SlateNode = this;
	// UpdateGraphNode();

	for (auto InputPin : InputPins)
	{
		InputPin->SetShowLabel(false);
	}
	for (auto OutputPin : OutputPins)
	{
		OutputPin->SetShowLabel(false);
	}
}

// static constexpr float ThumbnailIconSize = 32.0f;
// static constexpr uint32 ThumbnailIconResolution = 64;
//
// TSharedRef<SWidget> SMGNode_Montage::CreateNodeContentArea()
// {
// 	// return SMGNode::CreateNodeContentArea();
// 	
// 	UMGEdNode* MGGraphEdNode = Cast<UMGEdNode>(GraphNode);
// 	if (!MGGraphEdNode )
// 	{
// 	}
// 	
// 	UMGNode_Montage* AnimNode = Cast<UMGNode_Montage>(MGGraphEdNode->RuntimeNode);
// 	if (!AnimNode || !AnimNode->Montage)
// 	{
// 		return SMGNode::CreateNodeContentArea();
// 	}
// 	
// 	FAssetThumbnailConfig ThumbnailConfig;
// 	
// 	if (FSlateApplication::Get().InKismetDebuggingMode())
// 	{
// 		ThumbnailConfig.bForceGenericThumbnail = true;
// 	}
// 	
// 	TSharedPtr<FAssetThumbnail> Thumb = MakeShared<FAssetThumbnail>(AnimNode->Montage,
// 	                                                                ThumbnailIconResolution,
// 	                                                                ThumbnailIconResolution,
// 	                                                                UThumbnailManager::Get().GetSharedThumbnailPool());
// 	
// 	return SNew(SBorder)
// 		.BorderImage(FAppStyle::GetBrush("NoBorder"))
// 		.HAlign(HAlign_Fill)
// 		.VAlign(VAlign_Fill)
// 		.Padding(FMargin(0, 3))
// 		[
// 			SNew(SHorizontalBox)
// 			+ SHorizontalBox::Slot()
// 			.HAlign(HAlign_Left)
// 			.FillWidth(1.0f)
// 			[
// 				// LEFT
// 				SAssignNew(LeftNodeBox, SVerticalBox)
// 			]
// 			+ SHorizontalBox::Slot()
// 			.AutoWidth()
// 			.HAlign(HAlign_Center)
// 			[
// 				SNew(SBox)
// 				.MaxDesiredHeight(ThumbnailIconSize)
// 				.MaxDesiredWidth(ThumbnailIconSize)
// 				[
// 					Thumb->MakeThumbnailWidget(ThumbnailConfig)
// 				]
// 			]
// 	
// 			+ SHorizontalBox::Slot()
// 			.AutoWidth()
// 			.HAlign(HAlign_Right)
// 			[
// 				// RIGHT
// 				SAssignNew(RightNodeBox, SVerticalBox)
// 			]
// 		];
// }


const bool SMGNode_Montage::IsTitleVisible() const
{
	return false;
}
//
// void SMGNode_Montage::UpdateGraphNode()
// {
// 	InputPins.Empty();
// 	OutputPins.Empty();
//
// 	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
// 	RightNodeBox.Reset();
// 	LeftNodeBox.Reset();
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
// 	TSharedPtr<SVerticalBox> MainVerticalBox;
// 	SetupErrorReporting();
//
// 	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
//
// 	// Get node icon
// 	IconColor = FLinearColor::White;
// 	const FSlateBrush* IconBrush = nullptr;
// 	if (GraphNode != NULL && GraphNode->ShowPaletteIconOnNode())
// 	{
// 		IconBrush = GraphNode->GetIconAndTint(IconColor).GetOptionalIcon();
// 	}
//
// 	TSharedRef<SOverlay> DefaultTitleAreaWidget =
// 		SNew(SOverlay)
//
// 		+ SOverlay::Slot()
// 		.HAlign(HAlign_Fill)
// 		.VAlign(VAlign_Center)
// 		[
// 			SNew(SHorizontalBox)
// 			+ SHorizontalBox::Slot()
// 			.HAlign(HAlign_Fill)
// 			[
// 				SNew(SBorder)
// 				             .BorderImage(FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.Node.Title.Overlay"))
// 				// .Padding(TitleBorderMargin)
// 				             .Padding(FMargin(1.f, 1.f, 1.f, 0.f))
// 				             .BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
// 				[
// 					SNew(SBorder)
// 					.BorderImage(FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.Node.Title.Overlay.Body"))
// 					.Padding(FMargin(TitleBorderMargin))
//
// 					.HAlign(HAlign_Fill)
// 					.VAlign(VAlign_Fill)
// 					.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
// 					[
//
// 						SNew(SHorizontalBox)
// 						+ SHorizontalBox::Slot()
// 						  .VAlign(VAlign_Center)
// 						  // .Padding(FMargin(0.f))
// 						  .Padding(FMargin(1.f, 1.f, 1.f, 0.f))
// 						  .AutoWidth()
// 						[
// 							SNew(SImage)
// 							.Image(IconBrush)
// 							.ColorAndOpacity(this, &SGraphNode::GetNodeTitleIconColor)
// 						]
// 						+ SHorizontalBox::Slot()
// 						.VAlign(VAlign_Center)
// 						.HAlign(HAlign_Center)
// 						[
// 							SNew(SVerticalBox)
// 							+ SVerticalBox::Slot()
// 							.VAlign(VAlign_Center)
// 							.HAlign(HAlign_Center)
// 							.AutoHeight()
// 							[
// 								CreateTitleWidget(NodeTitle)
// 							]
// 							+ SVerticalBox::Slot()
// 							.VAlign(VAlign_Center)
// 							.HAlign(HAlign_Center)
// 							.AutoHeight()
// 							[
// 								NodeTitle.ToSharedRef()
// 							]
// 						]
// 					]
// 				]
// 			]
// 			+ SHorizontalBox::Slot()
// 			.HAlign(HAlign_Right)
// 			.VAlign(VAlign_Center)
// 			.Padding(0, 0, 0, 0)
// 			.AutoWidth()
// 			[
// 				CreateTitleRightWidget()
// 			]
//
// 		];
//
// 	if (IsTitleVisible())
// 	{
// 		SetDefaultTitleAreaWidget(DefaultTitleAreaWidget);
// 	}
//
// 	const TSharedRef<SWidget> TitleAreaWidget =
// 		SNew(SLevelOfDetailBranchNode)
// 		.Visibility_Lambda([this]()
// 		{
// 			return IsTitleVisible() ? EVisibility::Visible : EVisibility::Collapsed;
// 		})
// 		.UseLowDetailSlot(this, &SMGNode::UseLowDetailNodeTitles)
// 		.LowDetail()
// 		[
// 			SNew(SBorder)
// 			.BorderImage(FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.Node.Title"))
// 			.Padding(FMargin(75.0f, 22.0f))
// 			.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
// 		]
// 		.HighDetail()
// 		[
// 			DefaultTitleAreaWidget
// 		];
//
//
// 	if (!SWidget::GetToolTip().IsValid())
// 	{
// 		TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip(
// 			TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(),
// 			GraphNode->GetDocumentationExcerptName());
// 		SetToolTip(DefaultToolTip);
// 	}
//
//
// 	TSharedPtr<SVerticalBox> InnerVerticalBox;
// 	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
//
//
// 	InnerVerticalBox = SNew(SVerticalBox)
// 		+ SVerticalBox::Slot()
// 		  .AutoHeight()
// 		  .HAlign(HAlign_Fill)
// 		  .VAlign(VAlign_Top)
// 		  // .Padding(Settings->GetNonPinNodeBodyPadding())
// 		  .Padding(FMargin(0.f))
// 		[
// 			TitleAreaWidget
//
// 		]
//
// 		+ SVerticalBox::Slot()
// 		.AutoHeight()
// 		.HAlign(HAlign_Fill)
// 		.VAlign(VAlign_Top)
// 		.Padding(FMargin(0.f))
// 		[
// 			CreateNodeContentArea()
// 		];
//
// 	// TSharedPtr<SWidget> EnabledStateWidget = GetEnabledStateWidget();
// 	// if (EnabledStateWidget.IsValid())
// 	// {
// 	// 	InnerVerticalBox->AddSlot()
// 	// 	                .AutoHeight()
// 	// 	                .HAlign(HAlign_Fill)
// 	// 	                .VAlign(VAlign_Top)
// 	// 	                .Padding(FMargin(2, 0))
// 	// 	[
// 	// 		EnabledStateWidget.ToSharedRef()
// 	// 	];
// 	// }
// 	//
// 	// InnerVerticalBox->AddSlot()
// 	//                 .AutoHeight()
// 	//                 .Padding(Settings->GetNonPinNodeBodyPadding())
// 	// [
// 	// 	ErrorReporting->AsWidget()
// 	// ];
// 	//
// 	// InnerVerticalBox->AddSlot()
// 	//                 .AutoHeight()
// 	//                 .Padding(Settings->GetNonPinNodeBodyPadding())
// 	// [
// 	// 	VisualWarningReporting->AsWidget()
// 	// ];
//
//
// 	this->GetOrAddSlot(ENodeZone::Center)
// 	    .HAlign(HAlign_Center)
// 	    .VAlign(VAlign_Center)
// 	[
// 		SAssignNew(MainVerticalBox, SVerticalBox)
// 		+ SVerticalBox::Slot()
// 		.AutoHeight()
// 		[
// 			SNew(SOverlay)
// 			.AddMetaData<FGraphNodeMetaData>(TagMeta)
// 			+ SOverlay::Slot()
// 			.Padding(Settings->GetNonPinNodeBodyPadding())
// 			[
// 				SNew(SImage)
// 				.Image(GetNodeBodyBrush())
// 				.ColorAndOpacity(this, &SGraphNode::GetNodeTitleColor)
// 			]
// 			+ SOverlay::Slot()
// 			[
// 				InnerVerticalBox.ToSharedRef()
// 			]
// 		]
// 	];
//
//
//
// 	CreateBelowWidgetControls(MainVerticalBox);
// 	CreatePinWidgets();
// 	CreateInputSideAddButton(LeftNodeBox);
// 	CreateOutputSideAddButton(RightNodeBox);
// 	CreateBelowPinControls(InnerVerticalBox);
// 	CreateAdvancedViewArrow(InnerVerticalBox);
//
// // void SMGNode_Montage::UpdateGraphNode()
// // {
// 	// // if (auto MGEdGraph = Cast<UMontageGraphEdGraph>(GraphNode->GetGraph()))
// 	// // {
// 	// // 	if (MGEdGraph->GetEditingType() == FEditorMode::User)
// 	// // }
// 	// // SMGNode::UpdateGraphNode();
// 	// //
// 	//
// 	// InputPins.Empty();
// 	// OutputPins.Empty();
// 	//
// 	// // Reset variables that are going to be exposed, in case we are refreshing an already setup node.
// 	// RightNodeBox.Reset();
// 	// LeftNodeBox.Reset();
// 	//
// 	// TSharedPtr<SErrorText> ErrorText;
// 	// TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
// 	// constexpr FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f, 0.3f);
// 	// const FSlateBrush* NodeTypeIcon = GetNameIcon();
// 	//
// 	// const TSharedPtr<SVerticalBox> NodeContent = CreateNodeContent();
// 	//
// 	// ContentScale.Bind(this, &SGraphNode::GetContentScale);
// 	// GetOrAddSlot(ENodeZone::Center)
// 	// 	.HAlign(HAlign_Center)
// 	// 	.VAlign(VAlign_Center)
// 	// 	[
// 	// 		SNew(SBorder)
// 	// 		.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
// 	// 		.Padding(0)
// 	// 		.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
// 	// 		[
// 	// 			SNew(SOverlay)
// 	//
// 	// 			// PIN AREA
// 	// 			+ SOverlay::Slot()
// 	// 			.HAlign(HAlign_Fill)
// 	// 			.VAlign(VAlign_Fill)
// 	// 			[
// 	// 				SAssignNew(RightNodeBox, SVerticalBox)
// 	// 			]
// 	//
// 	// 			// STATE NAME AREA
// 	// 			+ SOverlay::Slot()
// 	// 			.HAlign(HAlign_Center)
// 	// 			.VAlign(VAlign_Center)
// 	// 			.Padding(10.0f)
// 	// 			[
// 	// 				SNew(SBorder)
// 	// 				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
// 	// 				.BorderBackgroundColor(TitleShadowColor)
// 	// 				.HAlign(HAlign_Center)
// 	// 				.VAlign(VAlign_Center)
// 	// 				.Visibility(EVisibility::SelfHitTestInvisible)
// 	// 				[
// 	// 					SNew(SHorizontalBox)
// 	// 					+ SHorizontalBox::Slot()
// 	// 					.AutoWidth()
// 	// 					[
// 	// 						// POPUP ERROR MESSAGE
// 	// 						SAssignNew(ErrorText, SErrorText)
// 	// 						.BackgroundColor(this, &SMGNode_Montage::GetErrorColor)
// 	// 						.ToolTipText(this, &SMGNode_Montage::GetErrorMsgToolTip)
// 	// 					]
// 	// 					+ SHorizontalBox::Slot()
// 	// 					.AutoWidth()
// 	// 					.VAlign(VAlign_Center)
// 	// 					[
// 	// 						SNew(SImage)
// 	// 						.Image(NodeTypeIcon)
// 	// 					]
// 	// 					+ SHorizontalBox::Slot()
// 	// 					.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
// 	// 					[
// 	// 						SNew(SVerticalBox)
// 	// 						+ SVerticalBox::Slot()
// 	// 						.AutoHeight()
// 	// 						[
// 	// 							SAssignNew(InlineEditableText, SInlineEditableTextBlock)
// 	// 							.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
// 	// 							.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
// 	// 							.OnVerifyTextChanged(this, &SMGNode_Montage::OnVerifyNameTextChanged)
// 	// 							.OnTextCommitted(this, &SMGNode_Montage::OnNameTextCommited)
// 	// 							.IsReadOnly(this, &SMGNode_Montage::IsNameReadOnly)
// 	// 							.IsSelected(this, &SMGNode_Montage::IsSelectedExclusively)
// 	// 						]
// 	// 						+ SVerticalBox::Slot()
// 	// 						.AutoHeight()
// 	// 						[
// 	// 							NodeTitle.ToSharedRef()
// 	// 						]
// 	// 					]
// 	// 				]
// 	// 			]
// 	// 		]
// 	// 	];
// 	//
// 	// // // Create comment bubble
// 	// // TSharedPtr<SCommentBubble> CommentBubble;
// 	// // const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;
// 	// //
// 	// // SAssignNew(CommentBubble, SCommentBubble)
// 	// // 	.GraphNode(GraphNode)
// 	// // 	.Text(this, &SGraphNode::GetNodeComment)
// 	// // 	.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
// 	// // 	.ColorAndOpacity(CommentColor)
// 	// // 	.AllowPinning(true)
// 	// // 	.EnableTitleBarBubble(true)
// 	// // 	.EnableBubbleCtrls(true)
// 	// // 	.GraphLOD(this, &SGraphNode::GetCurrentLOD)
// 	// // 	.IsGraphNodeHovered(this, &SGraphNode::IsHovered);
// 	// //
// 	// // GetOrAddSlot(ENodeZone::TopCenter)
// 	// // 	.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
// 	// // 	.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
// 	// // 	.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
// 	// // 	.VAlign(VAlign_Top)
// 	// // 	[
// 	// // 		CommentBubble.ToSharedRef()
// 	// // 	];
// 	// //
// 	// ErrorReporting = ErrorText;
// 	// ErrorColor = FAppStyle::GetColor("ErrorReporting.BackgroundColor");
// 	// //ErrorReporting->SetError(ErrorMsg);
// 	//
// 	//
// 	// CreateBelowWidgetControls(NodeContent);
// 	// // CreatePinWidgets();
// 	// // CreateInputSideAddButton(LeftNodeBox);
// 	// // CreateOutputSideAddButton(RightNodeBox);
// 	// CreateBelowPinControls(NodeContent);
// 	// CreateAdvancedViewArrow(NodeContent);
// 	// // CreatePinWidgets();
// // }
//
// //
// // void SMGNode_Montage::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
// // {
// // 	// SGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
// // 	//
// // 	// if (UMGEdNode* MGGraphEdNode = Cast<UMGEdNode>(GraphNode))
// // 	// {
// // 	// 	MGGraphEdNode->UpdateTime(InDeltaTime);
// // 	// }
// // }
// //
// // void SMGNode_Montage::UpdateGraphNode()
// // {
// //
// // }
// //
// // void SMGNode_Montage::CreatePinWidgets()
// // {
// // 	// UMGEdNode* StateNode = CastChecked<UMGEdNode>(GraphNode);
// // 	//
// // 	// UEdGraphPin* CurPin = StateNode->GetOutputPin();
// // 	// if (!CurPin->bHidden)
// // 	// {
// // 	// 	TSharedPtr<SGraphPin> NewPin = SNew(SMontageGraphPin, CurPin);
// // 	//
// // 	// 	AddPin(NewPin.ToSharedRef());
// // 	// }
// // }
// //
// // void SMGNode_Montage::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
// // {
// // 	PinToAdd->SetOwner(SharedThis(this));
// //
// // 	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
// // 	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
// // 	if (bAdvancedParameter)
// // 	{
// // 		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
// // 	}
// //
// // 	const FName PinName = PinObj ? PinObj->PinName : NAME_None;
// //
// // 	RightNodeBox->AddSlot()
// // 	            .HAlign(HAlign_Fill)
// // 	            .VAlign(VAlign_Fill)
// // 	            .FillHeight(1.0f)
// // 	[
// // 		PinToAdd
// // 	];
// // 	OutputPins.Add(PinToAdd);
// // }
// //
// // bool SMGNode_Montage::IsNameReadOnly() const
// // {
// // 	UMGEdNode* EdNode_Node = Cast<UMGEdNode>(GraphNode);
// // 	check(EdNode_Node != nullptr);
// //
// // 	return (!EdNode_Node->RuntimeNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
// // }
// //
// // int32 SMGNode_Montage::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
// //                               const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
// //                               int32 LayerId,
// //                               const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
// // {
// // 	if (GraphNode)
// // 	{
// // 		uint8 DebugMode = MG_DEBUG_NONE;
// // 		auto MontageEdGraph = Cast<UMontageGraphEdGraph>(GraphNode->GetGraph());
// // 		if (MontageEdGraph && MontageEdGraph->Debugger.IsValid())
// // 		{
// // 			FMontageGraphDebugger* Debugger = MontageEdGraph->Debugger.Get();
// // 			if (Debugger && Debugger->IsDebuggerReady())
// // 			{
// // 				DebugMode = Debugger->GetDebugMode();
// // 			}
// // 		}
// //
// // 		if (DebugMode == MG_DEBUG_TAGS)
// // 		{
// // 			if (UMGEdNode* MontageNode = CastChecked<UMGEdNode>(GraphNode))
// // 			{
// // 				UMGNode_Montage* AnimNode = CastChecked<UMGNode_Montage>(
// // 					MontageNode->RuntimeNode);
// // 				if (!AnimNode)
// // 				{
// // 					return SGraphNode::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
// // 					                           InWidgetStyle,
// // 					                           bParentEnabled);
// // 				}
// //
// // 				// for(auto Tag : )
// //
// // 				FSlateFontInfo FontInfo = FMontageGraphEditorStyle::Get().GetFontStyle("HB.Font.Small");
// // 				FontInfo.OutlineSettings.OutlineSize = 1;
// // 				FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
// // 				FontInfo.Size = 10;
// //
// // 				// FString FrameString = FString::FromInt(MontageNode->NodeIndex);
// //
// // 				FString FrameString = AnimNode->AggregatedTags.ToStringSimple();
// // 				FrameString = FrameString.Replace(TEXT(","),TEXT("\n"));
// //
// // 				FVector2d TextOffset = FVector2d(0.f, -15.f);
// //
// // 				// Adjust padding for the box
// // 				FMargin Padding(4.0f);
// //
// // 				const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->
// // 					GetFontMeasureService();
// // 				const FVector2D TextSize = FontMeasureService->Measure(FrameString, FontInfo);
// //
// // 				// Calculate the box size and position
// // 				FVector2D BoxPosition = TextOffset - FVector2D(Padding.Left, Padding.Top);
// // 				FVector2D BoxSize = TextSize + FVector2D(Padding.GetTotalSpaceAlong<Orient_Horizontal>(),
// // 				                                         Padding.GetTotalSpaceAlong<Orient_Vertical>());
// //
// // 				FPaintGeometry TextGeometry =
// // 					AllottedGeometry.ToPaintGeometry(BoxPosition, BoxSize);
// //
// // 				static const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("GenericWhiteBox");
// //
// // 				// Draw solid background
// // 				FSlateDrawElement::MakeBox(
// // 					OutDrawElements,
// // 					LayerId + 50,
// // 					TextGeometry,
// // 					WhiteBrush,
// // 					ESlateDrawEffect::None,
// // 					FLinearColor(FColor(98, 98, 98))
// // 				);
// //
// // 				FSlateDrawElement::MakeText(
// // 					OutDrawElements,
// // 					LayerId + 50,
// // 					TextGeometry,
// // 					FrameString,
// // 					FontInfo,
// // 					ESlateDrawEffect::None,
// // 					FLinearColor::Green);
// // 			}
// // 		}
// // 	}
// //
// // 	return SGraphNode::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
// // 	                           bParentEnabled);
// // }
// //
// // void SMGNode_Montage::OnNameTextCommited(const FText& InText, const ETextCommit::Type CommitInfo)
// // {
// // 	SGraphNode::OnNameTextCommited(InText, CommitInfo);
// //
// // 	UMGEdNode* MyNode = CastChecked<UMGEdNode>(GraphNode);
// //
// // 	if (MyNode != nullptr && MyNode->RuntimeNode != nullptr)
// // 	{
// // 		const FScopedTransaction Transaction(
// // 			LOCTEXT("MontageGraphEditorRenameNode", "MontageGraph Editor: Rename Node"));
// // 		MyNode->Modify();
// // 		MyNode->RuntimeNode->Modify();
// // 		MyNode->RuntimeNode->SetNodeTitle(InText);
// // 		UpdateGraphNode();
// // 	}
// // }
// //
// // FSlateColor SMGNode_Montage::GetBorderBackgroundColor() const
// // {
// // }
// //
// // EVisibility SMGNode_Montage::GetDragOverMarkerVisibility() const
// // {
// // 	return EVisibility::Visible;
// // }
// //
// // const FSlateBrush* SMGNode_Montage::GetNameIcon() const
// // {
// // 	return FMontageGraphEditorStyle::Get().GetBrush(TEXT("MontageGraph.Icon"));
// // }
// //
// //
// // // ReSharper disable once CppMemberFunctionMayBeConst
// // void SMGNode_Montage::SetErrorText(const FText InErrorText)
// // {
// // 	if (ErrorReporting)
// // 	{
// // 		ErrorReporting->SetError(InErrorText);
// // 	}
// // }
// //
// // FText SMGNode_Montage::GetMontageText()
// // {
// // 	return FText::FromString("");
// // }
// //
// TSharedPtr<SVerticalBox> SMGNode_Montage::CreateNodeContent()
// {
// 	
// 	TSharedPtr<SVerticalBox> Content = SNew(SVerticalBox);
// 	const TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
// 	
// 	Content->AddSlot()
// 	       .AutoHeight()
// 	       .HAlign(HAlign_Fill)
// 	[
// 		SAssignNew(InlineEditableText, SInlineEditableTextBlock)
// 		.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
// 		.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
// 		.OnVerifyTextChanged(this, &SMGNode_Montage::OnVerifyNameTextChanged)
// 		.OnTextCommitted(this, &SMGNode_Montage::OnNameTextCommited)
// 		.IsReadOnly(this, &SMGNode_Montage::IsNameReadOnly)
// 		.IsSelected(this, &SMGNode_Montage::IsSelectedExclusively)
// 		.Justification(ETextJustify::Center)
// 	];
// 	
// 	Content->AddSlot()
// 	       .AutoHeight()
// 	[
// 		NodeTitle.ToSharedRef()
// 	];
// 	
// 	UMGEdNode* MGGraphEdNode = Cast<UMGEdNode>(GraphNode);
// 	if (!MGGraphEdNode || !MGGraphEdNode->RuntimeNode)
// 	{
// 		return Content;
// 	}
// 	
// 	const FMargin RowPadding = FMargin(0.f, 0.f, 0.f, 0.f);
// 	const FMargin TopRowPadding = FMargin(0.f, 0.f, 0.f, 0.f);
// 	const FMargin BottomRowPadding = FMargin(0.f, 0.f, 0.f, 0.f);
// 	//
// 	// Content->AddSlot()
// 	//        .Padding(TopRowPadding)
// 	//        .AutoHeight()
// 	// [
// 	// 	SNew(SMontageGraphNodeRow, MGGraphEdNode ->RuntimeNode)
// 	// 	                                                  .IsTransparent(false)
// 	// 	// .LabelText(GraphNode->RuntimeNode->GetAnimAssetLabel())
// 	// 	// .LabelTooltipText(GraphNode->RuntimeNode->GetAnimAssetLabelTooltip())
// 	// 	// .ValueTooltipText(GraphNode->RuntimeNode->GetAnimAssetLabelTooltip())
// 	// 	                                                  // .OnGetValueText(FText::FromString(TEXT("LOL")))
// 	// ];
//
// 	
// 	Content->AddSlot()
// 	       .Padding(RowPadding)
// 	       .AutoHeight()
// 	[
// 		SNew(SMontageGraphNodeRow, MGGraphEdNode->RuntimeNode)
// 		.IsTransparent(false)
// 		.LabelText(LOCTEXT("PlayRate", "Rate"))
// 		.LabelTooltipText(LOCTEXT("PlayRateTooltip", "Montage Play Rate"))
// 		.ValueTooltipText(LOCTEXT("PlayRateValueTooltip", "Montage Play Rate"))
// 		// .OnGetValueText(FACEOnGetValueText::CreateSP(this, &SMontageGraphNode::GetMontagePlayRateText))
// 	];
//
// 	// if (GraphNode->RuntimeNode)
// 	// {
// 	// 	Content->AddSlot()
// 	// 	       .Padding(RowPadding)
// 	// 	       .AutoHeight()
// 	// 	[
// 	// 		SNew(SMontageGraphNodeRow, GraphNode->RuntimeNode)
// 	// 		.IsTransparent(false)
// 	// 		.LabelText(LOCTEXT("StartSection", "Montage Start Section"))
// 	// 		.LabelTooltipText(LOCTEXT("StartSection", "Montage Start Section"))
// 	// 		.ValueTooltipText(LOCTEXT("StartSection", "Montage Start Section"))
// 	// 		.OnGetValueText(FACEOnGetValueText::CreateSP(this, &SMontageGraphNode::GetMontageStartSectionName))
// 	// 		.Visibility(this, &SMontageGraphNode::GetMontageStartSectionNameVisibility)
// 	// 	];
// 	// }
//
// 	Content->AddSlot()
// 	       .Padding(RowPadding)
// 	       .AutoHeight()
// 	[
// 		SNew(SMontageGraphNodeRow, MGGraphEdNode->RuntimeNode)
// 		.IsTransparent(false)
// 		.LabelText(LOCTEXT("EffectCost", "Effect Cost"))
// 		.LabelTooltipText(LOCTEXT("EffectCost", "Effect Cost"))
// 		.ValueTooltipText(LOCTEXT("EffectCost", "Effect Cost"))
// 		// .OnGetValueText(FACEOnGetValueText::CreateSP(this, &SMontageGraphNode::GetGameplayEffectCostText))
// 		// .Visibility(this, &SMontageGraphNode::GetGameplayEffectCostTextVisibility)
// 	];
//
// 	
// 	//
// 	// if (MGGraphEdNode->RuntimeNode->IsHBActionSubclassedInBlueprint())
// 	// {
// 	// 	FString Classname = GetNameSafe(MGGraphEdNode->RuntimeNode->GetClass());
// 	// 	Classname.RemoveFromEnd("_C");
// 	//
// 	// 	Content->AddSlot()
// 	// 	       .Padding(BottomRowPadding)
// 	// 	       .AutoHeight()
// 	// 	[
// 	// 		SNew(SMontageGraphNodeRow, MGGraphEdNode ->RuntimeNode)
// 	// 		                                                       .IsTransparent(true)
// 	// 		// .LabelText(LOCTEXT("NodeClass", "Node Class"))
// 	// 		                                                       .LabelTooltipText(LOCTEXT("NodeClass", "Node Class"))
// 	// 		                                                       .ValueTooltipText(LOCTEXT("NodeClass", "Node Class"))
// 	// 		                                                       .OnGetValueText(FACEOnGetValueText::CreateLambda(
// 	// 			                                                       [Classname]()
// 	// 			                                                       {
// 	// 				                                                       return FText::FromString(*Classname);
// 	// 			                                                       }))
// 	// 	];
// 	// }
//
// 	return Content;
// }
// //
// // //
// // // EVisibility SMontageGraphNode::GetGameplayEffectCostTextVisibility() const
// // // {
// // // 	const TSubclassOf<UGameplayEffect> CostGE = GraphNode->RuntimeNode->CostGameplayEffect;
// // // 	return IsValid(CostGE) ? EVisibility::Visible : EVisibility::Collapsed;
// // // }
// // //
// // // FText SMontageGraphNode::GetGameplayEffectCostText() const
// // // {
// // // 	const TSubclassOf<UGameplayEffect> CostGE = GraphNode->RuntimeNode->CostGameplayEffect;
// // // 	return FText::FromString(CostGE ? CostGE->GetName() : TEXT("NONE"));
// // // }
// //
// //
// // FText SMGNode_Montage::GetMontageStartSectionName() const
// // {
// // 	FName StartSectionName = NAME_None;
// // 	return FText::FromName(StartSectionName);
// // }
// //
// //
// // EVisibility SMGNode_Montage::GetMontageStartSectionNameVisibility() const
// // {
// // 	UMGEdNode* MGGraphEdNode = Cast<UMGEdNode>(GraphNode);
// // 	if (!MGGraphEdNode || !MGGraphEdNode->RuntimeNode)
// // 	{
// // 		return EVisibility::Collapsed;
// // 	}
// //
// // 	const FName StartSectionName;
// // 	return StartSectionName != NAME_None ? EVisibility::Visible : EVisibility::Collapsed;
// // }
// //
// // FText SMGNode_Montage::GetErrorText() const
// // {
// // 	return LOCTEXT("ErrorText", "Test Error Yo");
// // }
// //
// //
#undef LOCTEXT_NAMESPACE
