#include "SHBMontageGraphEntryPin.h"

#include "MontageGraphEditorStyles.h"

void SHBMontageGraphEntryPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
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
		.BorderImage(this, &SHBMontageGraphEntryPin::GetPinBorder)
		.BorderBackgroundColor(this, &SHBMontageGraphEntryPin::GetPinColor)
		.OnMouseButtonDown(this, &SHBMontageGraphEntryPin::OnPinMouseDown)
		.Cursor(this, &SHBMontageGraphEntryPin::GetPinCursor)
		.DesiredSizeScale(FVector2d(10.f, 10.f))
	);
}

TSharedRef<SWidget> SHBMontageGraphEntryPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SHBMontageGraphEntryPin::GetPinBorder() const
{
	FString StyleName = IsConnected()
		                    ? "HBEditor.MontageGraph.Pin.BackgroundConnected"
		                    : "HBEditor.MontageGraph.Pin.Background";
	
	StyleName.Append(IsHovered() ? "Hovered" : "");

	return FMontageGraphEditorStyles::Get().GetBrush(FName(*StyleName));
}
