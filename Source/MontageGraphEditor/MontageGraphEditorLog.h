#pragma once

#include "CoreMinimal.h"
MONTAGEGRAPHEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogMontageGraphEditor,
                                                  Display,
                                                  All);

#define MontageGraph_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogMontageGraphEditor, Verbosity, Format, ##__VA_ARGS__); \
}