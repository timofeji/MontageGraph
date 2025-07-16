
#include "MontageGraphEditorTypes.h"

// const FName FMontageGraphEditorModes::MontageGraphPersonaModeID("MontageGraph_MontageGraphPersonaMode");
// const FName FMontageGraphEditorModes::MontageGraphDefaultModeID("MontageGraph_MontageGraphDefaultMode");

const FName UMontageGraphPinNames::PinCategory_EntryOut("Entry");
const FName UMontageGraphPinNames::PinCategory_MultipleNodes("MultipleNodes");
const FName UMontageGraphPinNames::PinCategory_Transition("Transition");
const FName UMontageGraphPinNames::PinCategory_Edge("Edge");

const FName UMontageGraphPinNames::PinName_In("Input");
const FName UMontageGraphPinNames::PinName_Out("Output");

UMontageGraphPinNames::UMontageGraphPinNames(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}
