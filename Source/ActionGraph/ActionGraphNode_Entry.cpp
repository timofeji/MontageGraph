#include "ActionGraphNode_Entry.h"

#define LOCTEXT_NAMESPACE "HBActionGraphNodeEntry"

FText UActionGraphNode_Entry::GetNodeTitle() const
{
	return LOCTEXT("ActionGraphEntryNode", "Action Graph Entry Node");
}

#undef LOCTEXT_NAMESPACE
