// Created by Timofej Jermolaev, All rights reserved . 


#include "SHBMontageGraphNodeRow.h"

#include "EditorStyleSet.h"
#include "SlateOptMacros.h"
#include "MontageGraph/MontageGraphNode.h"

#define LOCTEXT_NAMESPACE "SACEGraphNode"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SHBMontageGraphNodeRow::Construct(const FArguments& InArgs, UMontageGraphNode* InRuntimeNode)
{
	check(InRuntimeNode);
	RuntimeNode = InRuntimeNode;
	LabelText = InArgs._LabelText;
	LabelTooltipText = InArgs._LabelTooltipText;
	ValueTooltipText = InArgs._ValueTooltipText;
	OnGetValueText = InArgs._OnGetValueText;
	bIsTransparent = InArgs._IsTransparent;

	const FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f, bIsTransparent ? 0.f : 0.25f);

	const FSlateFontInfo FontStyle = FCoreStyle::GetDefaultFontStyle(
		bIsTransparent ? "Italic" : "Regular",
		bIsTransparent ? 7 : 9
	);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
		.BorderBackgroundColor(TitleShadowColor)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Visibility(EVisibility::SelfHitTestInvisible)
		.Padding(4.f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.AutoWidth()
			[
				// Label
				SNew(STextBlock)
				.Text(LabelText)
				.Font(FontStyle)
				.ToolTipText(LabelTooltipText)
			]
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.FillWidth(1.f)
			.Padding(FMargin(16.f, 0.f, 4.f, 0.f))
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Right)
				.Font(FontStyle)
				.Text(this, &SHBMontageGraphNodeRow::GetValueText)
				.ToolTipText(ValueTooltipText)
			]
		]
	];
}

FText SHBMontageGraphNodeRow::GetValueText() const
{
	return OnGetValueText.IsBound() ? OnGetValueText.Execute() : LOCTEXT("NotImplemented", "NotImplemented: Please provide OnGetValueText delegate");
	// return FText::FromString(FString::SanitizeFloat(RuntimeNode->MontagePlayRate));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
