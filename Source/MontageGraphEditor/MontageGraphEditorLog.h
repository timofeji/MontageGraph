#pragma once

#include "CoreMinimal.h"


MONTAGEGRAPHEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogMontageGraphEditor, Log, All);
MONTAGEGRAPHEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogMontageGraphEditorError, Error, All)

#define MG_LOG(Msg, ...) \
{ \
    UE_LOG(LogMontageGraphEditor, Log,  TEXT("[%s]:%s" ), *FString(__FUNCTION__), *FString::Printf( TEXT(Msg), ##__VA_ARGS__ )); \
}


#define MG_ERROR(Msg, ...) \
{ \
    UE_LOG(LogMontageGraphEditorError, Log,  TEXT("[%s]:%s" ), *FString(__FUNCTION__), *FString::Printf( TEXT(Msg), ##__VA_ARGS__ )); \
}

