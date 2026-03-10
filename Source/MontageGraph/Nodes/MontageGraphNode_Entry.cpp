#include "MontageGraphNode_Entry.h"

#define LOCTEXT_NAMESPACE "MontageGraphNodeEntry"

#if WITH_EDITOR
FText UMontageGraphNode_Entry::GetNodeTitle() const
{
	return LOCTEXT("MontageGraphEntryNode", "Montage Graph Entry Node");
}
#endif

#undef LOCTEXT_NAMESPACE
