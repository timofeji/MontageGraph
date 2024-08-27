#include "SHBActionGraphEntryPin.h"

#include "ActionGraphEditorStyles.h"

void SHBActionGraphEntryPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
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
		.BorderImage(this, &SHBActionGraphEntryPin::GetPinBorder)
		.BorderBackgroundColor(this, &SHBActionGraphEntryPin::GetPinColor)
		.OnMouseButtonDown(this, &SHBActionGraphEntryPin::OnPinMouseDown)
		.Cursor(this, &SHBActionGraphEntryPin::GetPinCursor)
		.DesiredSizeScale(FVector2d(10.f, 10.f))
	);
}

TSharedRef<SWidget> SHBActionGraphEntryPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SHBActionGraphEntryPin::GetPinBorder() const
{
	FString StyleName = IsConnected()
		                    ? "HBEditor.ActionGraph.Pin.BackgroundConnected"
		                    : "HBEditor.ActionGraph.Pin.Background";
	
	StyleName.Append(IsHovered() ? "Hovered" : "");

	return FActionGraphEditorStyles::Get().GetBrush(FName(*StyleName));
}
