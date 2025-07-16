#include "MGEdNode_Conduit.h"

#include "MontageGraphEditorStyle.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/Nodes/MGNode_Conduit.h"

#define LOCTEXT_NAMESPACE "MGEdNode_Conduit"

UMGEdNode_Conduit::UMGEdNode_Conduit()
{
	bCanRenameNode = false;
}

FLinearColor UMGEdNode_Conduit::GetNodeTitleColor() const
{
	return FLinearColor(FColor::FromHex(TEXT("#FAF8FFFF")));
}

FSlateIcon UMGEdNode_Conduit::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Node.Icon.Conduit");
}




FText UMGEdNode_Conduit::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("ConduitNodeTitle", "-|-");
}

FText UMGEdNode_Conduit::GetTooltipText() const
{
	return LOCTEXT("ConduitNodeTooltip",
	               "This is a Conduit, which allows branching out beginning of Montage Graph based on an initial Transition Input");
}

void UMGEdNode_Conduit::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}
#undef LOCTEXT_NAMESPACE
