#include "MontageGraphEditorSettings.h"

UMontageGraphEditorSettings::UMontageGraphEditorSettings()
{
	MontageNodeColor  = FLinearColor(100.f / 255.f, 100.f / 255.f, 1.f, 1.f);
	SequenceNodeColor = FLinearColor(80.f / 255.f, 123.f / 255.f, 72.f / 255.f, 1.f);

	DebugSelectedColor  = FLinearColor(0.95f, 0.9f, 0.9f);
	DebugEvaluatedColor = FLinearColor(1.00f, 0.71f, 0.00f, 1.00f);
	DebugErrorColor     = FLinearColor(0.95f, 0.2f, 0.1f);
	
	DebugFadeTime    = 0.75f;

	ContentMargin          = FMargin(8.f);
	ContentInternalPadding = FMargin(12.f, 12.f);

	IconSize = 48.f;

	DynamicMontageSlotName = TEXT("DefaultSlot");
}