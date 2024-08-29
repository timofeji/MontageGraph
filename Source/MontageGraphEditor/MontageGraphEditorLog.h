#pragma once

#include "CoreMinimal.h"
MONTAGEGRAPHEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogMontageGraphEditor,
                                                  Error,
                                                  All);

#define MG_ERROR(Verbosity, Format, ...) \
{ \
    UE_LOG(LogMontageGraphEditor, Verbosity, Format, ##__VA_ARGS__); \
}