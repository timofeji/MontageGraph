
#include "ActionGraphEditorTypes.h"

// const FName FHBActionGraphEditorModes::HBActionGraphPersonaModeID("HBActionGraph_HBActionGraphPersonaMode");
// const FName FHBActionGraphEditorModes::HBActionGraphDefaultModeID("HBActionGraph_HBActionGraphDefaultMode");

const FName UActionGraphPinNames::PinCategory_EntryOut("Entry");
const FName UActionGraphPinNames::PinCategory_MultipleNodes("MultipleNodes");
const FName UActionGraphPinNames::PinCategory_Transition("Transition");
const FName UActionGraphPinNames::PinCategory_Edge("Edge");

const FName UActionGraphPinNames::PinName_In("Input");
const FName UActionGraphPinNames::PinName_Out("Output");

UActionGraphPinNames::UActionGraphPinNames(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}
