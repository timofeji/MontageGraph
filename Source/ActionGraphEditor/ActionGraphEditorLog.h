#pragma once

#include "CoreMinimal.h"
ACTIONGRAPHEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogActionGraphEditor,
                                                  Display,
                                                  All);

#define ACTIONGRAPH_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogActionGraphEditor, Verbosity, Format, ##__VA_ARGS__); \
}