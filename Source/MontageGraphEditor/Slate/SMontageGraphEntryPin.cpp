#include "SMontageGraphEntryPin.h"

#include "MontageGraphEditorStyles.h"

void SMontageGraphEntryPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
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
		.BorderImage(this, &SMontageGraphEntryPin::GetPinBorder)
		.BorderBackgroundColor(this, &SMontageGraphEntryPin::GetPinColor)
		.OnMouseButtonDown(this, &SMontageGraphEntryPin::OnPinMouseDown)
		.Cursor(this, &SMontageGraphEntryPin::GetPinCursor)
		.DesiredSizeScale(FVector2d(10.f, 10.f))
	);
}

TSharedRef<SWidget> SMontageGraphEntryPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SMontageGraphEntryPin::GetPinBorder() const
{
	FString StyleName = IsConnected()
		                    ? "HBEditor.MontageGraph.Pin.BackgroundConnected"
		                    : "HBEditor.MontageGraph.Pin.Background";
	
	StyleName.Append(IsHovered() ? "Hovered" : "");

	return FMontageGraphEditorStyles::Get().GetBrush(FName(*StyleName));
}
