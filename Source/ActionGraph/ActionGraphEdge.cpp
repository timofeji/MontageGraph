// Created by Timofej Jermolaev, All rights reserved . 


#include "ActionGraphEdge.h"

#if WITH_EDITOR
void UActionGraphEdge::SetNodeTitle(const FText& InTitle)
{
	NodeTitle = InTitle;
}
#endif
