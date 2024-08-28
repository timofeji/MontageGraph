#include "MontageGraphNode_Entry.h"

#define LOCTEXT_NAMESPACE "HBMontageGraphNodeEntry"

FText UMontageGraphNode_Entry::GetNodeTitle() const
{
	return LOCTEXT("MontageGraphEntryNode", "Montage Graph Entry Node");
}

#undef LOCTEXT_NAMESPACE
