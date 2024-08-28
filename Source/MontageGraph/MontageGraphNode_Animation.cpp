
#include "MontageGraphNode_Animation.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphNodeSequence"

UMontageGraphNode_Animation::UMontageGraphNode_Animation()
{
#if WITH_EDITORONLY_DATA
	ContextMenuName = LOCTEXT("HBMontageGraphNodeSequence_ContextMenuName", "Montage Action Node");
#endif
}



bool UMontageGraphNode_Animation::SupportsAssetClass(UClass* AssetClass)
{
	return AssetClass->IsChildOf(UAnimSequence::StaticClass());
}

FText UMontageGraphNode_Animation::GetNodeTitle() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

#if WITH_EDITOR
#include "Preferences/PersonaOptions.h"
FText UMontageGraphNode_Animation::GetAnimAssetLabel() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

FText UMontageGraphNode_Animation::GetAnimAssetLabelTooltip() const
{
	return LOCTEXT("AnimAssetLabelTooltip", "Sequence");
}


FLinearColor UMontageGraphNode_Animation::GetBackgroundColor() const
{
	return  FLinearColor::Blue.Desaturate(0.24f);
}
#endif

#undef LOCTEXT_NAMESPACE
