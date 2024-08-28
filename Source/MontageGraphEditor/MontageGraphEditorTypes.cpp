
#include "MontageGraphEditorTypes.h"

// const FName FHBMontageGraphEditorModes::HBMontageGraphPersonaModeID("HBMontageGraph_HBMontageGraphPersonaMode");
// const FName FHBMontageGraphEditorModes::HBMontageGraphDefaultModeID("HBMontageGraph_HBMontageGraphDefaultMode");

const FName UMontageGraphPinNames::PinCategory_EntryOut("Entry");
const FName UMontageGraphPinNames::PinCategory_MultipleNodes("MultipleNodes");
const FName UMontageGraphPinNames::PinCategory_Transition("Transition");
const FName UMontageGraphPinNames::PinCategory_Edge("Edge");

const FName UMontageGraphPinNames::PinName_In("Input");
const FName UMontageGraphPinNames::PinName_Out("Output");

UMontageGraphPinNames::UMontageGraphPinNames(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}
