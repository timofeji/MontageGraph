// Created by Timofej Jermolaev, All rights reserved . 


#include "MontageGraphEditorCommands.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphBlueprintEditorCommands"

FMontageGraphEditorCommands::FMontageGraphEditorCommands() : TCommands<
	FMontageGraphEditorCommands>(
	"ACEBlueprintEditorCommands",
	NSLOCTEXT("Contexts", "ACEBlueprintEditorCommands", "Montage Graph Editor"),
	NAME_None,
	FAppStyle::GetAppStyleSetName())
{
}

void FMontageGraphEditorCommands::RegisterCommands()
{
	UI_COMMAND(AutoArrange, "Auto Arrange",
	           "Auto Arrange (Horizontally or Vertically, depending on last selected mode in dropdown)",
	           EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AutoArrangeVertical, "Auto Arrange (Vertically)",
	           "Auto Arrange (Vertically) - Re-arrange graph layout vertically (from top to bottom). Similar to Behavior Trees.",
	           EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AutoArrangeHorizontal, "Auto Arrange (Horizontally)",
	           "Auto Arrange (Horizontally) - Re-arrange graph layout horizontally (from left to right). This is far from perfect, but still handy. Will be revisited and improved upon in a future update.",
	           EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(EditClassDefaults, "Class Defaults", "Edit the initial values of your class.",
	           EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE
