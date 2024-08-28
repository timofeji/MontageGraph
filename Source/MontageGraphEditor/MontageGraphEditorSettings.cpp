#include "MontageGraphEditorSettings.h"

UMontageGraphEditorSettings::UMontageGraphEditorSettings()
{
	MontageNodeColor = FLinearColor(100.f / 255.f, 100.f / 255.f, 1.f, 1.f);
	SequenceNodeColor = FLinearColor(80.f / 255.f, 123.f / 255.f, 72.f / 255.f, 1.f);

	DebugActiveColor = FLinearColor(1.f, 0.6f, 0.35f, 1.f);
	DebugFadeTime = 0.75f;

	ContentMargin = FMargin(8.f);
	ContentInternalPadding = FMargin(12.f, 8.f);

	IconSize = 48.f;

	DynamicMontageSlotName = TEXT("DefaultSlot");

	// TODO: Don't forget to update path when plugin gets renamed
	// IconsDataTable = FSoftObjectPath(TEXT("/ComboGraph/Xelu_Icons/DT_ComboGraph_InputIconsMappings.DT_ComboGraph_InputIconsMappings"));

	// Make sure native combo window is added as default
	// NotifyStates.Add(UComboGraphANS_ComboWindow::StaticClass(), FComboGraphNotifyStateAutoSetup(0.25f, 0.75f));
}
