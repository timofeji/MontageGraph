
#include "MontageGraphNode_Action.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphNodeSequence"

UMontageGraphNode_Action::UMontageGraphNode_Action()
{
#if WITH_EDITORONLY_DATA
	ContextMenuName = LOCTEXT("HBMontageGraphNodeSequence_ContextMenuName", "Montage Action Node");
#endif
}



bool UMontageGraphNode_Action::SupportsAssetClass(UClass* AssetClass)
{
	return AssetClass->IsChildOf(UAnimSequence::StaticClass());
}

FText UMontageGraphNode_Action::GetNodeTitle() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

#if WITH_EDITOR
#include "Preferences/PersonaOptions.h"
FText UMontageGraphNode_Action::GetAnimAssetLabel() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

FText UMontageGraphNode_Action::GetAnimAssetLabelTooltip() const
{
	return LOCTEXT("AnimAssetLabelTooltip", "Sequence");
}


FLinearColor UMontageGraphNode_Action::GetBackgroundColor() const
{
	return  FLinearColor::Blue.Desaturate(0.24f);
}
#endif

#undef LOCTEXT_NAMESPACE
