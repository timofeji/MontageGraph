
#include  "MGNode_Montage.h"


#define LOCTEXT_NAMESPACE "MontageGraphNodeSequence"

UMGNode_Montage::UMGNode_Montage()
{
#if WITH_EDITORONLY_DATA
	ContextMenuName = LOCTEXT("MontageGraphNodeSequence_ContextMenuName", "Montage Action Node");
#endif
}



bool UMGNode_Montage::SupportsAssetClass(UClass* AssetClass)
{
	return AssetClass->IsChildOf(UAnimSequence::StaticClass());
}

FText UMGNode_Montage::GetNodeTitle() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

#if WITH_EDITOR
#include "Preferences/PersonaOptions.h"
FText UMGNode_Montage::GetAnimAssetLabel() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

FText UMGNode_Montage::GetAnimAssetLabelTooltip() const
{
	return LOCTEXT("AnimAssetLabelTooltip", "Sequence");
}


FLinearColor UMGNode_Montage::GetBackgroundColor() const
{
	return  FLinearColor::Blue.Desaturate(0.24f);
}
#endif

#undef LOCTEXT_NAMESPACE
