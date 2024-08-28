

#include "SHBMontageGraphSelectorOutputPin.h"

#include "MontageGraphEditor/MontageGraphEditorStyles.h"
#include "MontageGraphEditor/Graph/HBMontageGraphDragDropAction.h"

void SHBMontageGraphSelectorOutputPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SetCursor(EMouseCursor::Default);

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	// Set up a hover for pins that is tinted the color of the pin.
	SBorder::Construct(
		SBorder::FArguments()
	   .BorderImage(this, &SHBMontageGraphSelectorOutputPin::GetPinBorder)
	   .BorderBackgroundColor(this, &SHBMontageGraphSelectorOutputPin::GetPinColor)
	   .OnMouseButtonDown(this, &SHBMontageGraphSelectorOutputPin::OnPinMouseDown)
	   .Cursor(this, &SHBMontageGraphSelectorOutputPin::GetPinCursor)
	);
}

int32 SHBMontageGraphSelectorOutputPin::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                              const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                              int32 LayerId,
                                              const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const ISlateStyle& Style = FMontageGraphEditorStyles::Get();
	const FSlateBrush* BackgroundBrush = Style.GetBrush("HBEditor.MontageGraph.Entry");
	FSlateFontInfo FontInfo = Style.GetFontStyle("HB.Font.Small");
	FontInfo.OutlineSettings.OutlineSize = 1;
	FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
	
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(FVector2D(0, 0),
		                                 FVector2D(AllottedGeometry.Size.X, AllottedGeometry.Size.Y) * 0.1f),
		BackgroundBrush,
		ESlateDrawEffect::None
	);

	//
	// FSlateDrawElement::MakeText(
	// 	OutDrawElements,
	// 	LayerId + 2,
	// 	AllottedGeometry.ToPaintGeometry(FVector2D(0, 0),
	// 	                                 FVector2D(AllottedGeometry.Size.X, AllottedGeometry.Size.Y) * 0.1f),
	// 	FString::FromInt(TagIndex),
	// 	FontInfo);
	//
	return SGraphPin::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                          bParentEnabled);
}

TSharedRef<SWidget> SHBMontageGraphSelectorOutputPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

TSharedRef<FDragDropOperation> SHBMontageGraphSelectorOutputPin::SpawnPinDragEvent(
	const TSharedRef<SGraphPanel>& InGraphPanel, const TArray<TSharedRef<SGraphPin>>& InStartingPins)
{
	TArray<FGraphPinHandle> PinHandles;
	PinHandles.Reserve(InStartingPins.Num());

	// Since the graph can be refreshed and pins can be reconstructed/replaced behind the scenes, the DragDropOperation holds onto FGraphPinHandles
	// instead of direct widgets/graph-pins
	for (const TSharedRef<SGraphPin>& PinWidget : InStartingPins)
	{
		PinHandles.Add(PinWidget->GetPinObj());
	}

	return FHBMontageGraphDragDropAction::New(InGraphPanel, PinHandles);
}

const FSlateBrush* SHBMontageGraphSelectorOutputPin::GetPinBorder() const
{


	FString StyleName = IsConnected()
		                    ? "HBEditor.MontageGraph.Pin.BackgroundConnected"
		                    : "HBEditor.MontageGraph.Pin.Background";
	
	StyleName.Append(IsHovered() ? "Hovered" : "");

	return FMontageGraphEditorStyles::Get().GetBrush(FName(*StyleName));
}
