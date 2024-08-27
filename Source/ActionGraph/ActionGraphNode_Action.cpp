
#include "ActionGraphNode_Action.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"

#define LOCTEXT_NAMESPACE "HBActionGraphNodeSequence"

UActionGraphNode_Action::UActionGraphNode_Action()
{
#if WITH_EDITORONLY_DATA
	ContextMenuName = LOCTEXT("HBActionGraphNodeSequence_ContextMenuName", "Montage Action Node");
#endif
}



bool UActionGraphNode_Action::SupportsAssetClass(UClass* AssetClass)
{
	return AssetClass->IsChildOf(UAnimSequence::StaticClass());
}

FText UActionGraphNode_Action::GetNodeTitle() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

#if WITH_EDITOR
#include "Preferences/PersonaOptions.h"
FText UActionGraphNode_Action::GetAnimAssetLabel() const
{
	return LOCTEXT("AnimAssetLabel", "Sequence");
}

FText UActionGraphNode_Action::GetAnimAssetLabelTooltip() const
{
	return LOCTEXT("AnimAssetLabelTooltip", "Sequence");
}


FLinearColor UActionGraphNode_Action::GetBackgroundColor() const
{
	return  FLinearColor::Blue.Desaturate(0.24f);
}
#endif

#undef LOCTEXT_NAMESPACE
