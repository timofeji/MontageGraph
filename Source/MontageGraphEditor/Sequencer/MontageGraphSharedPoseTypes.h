#pragma once

#include "MontageGraphSharedPoseTypes.generated.h"

USTRUCT()
struct MONTAGEGRAPHEDITOR_API FSharedPoseHandle
{
	GENERATED_BODY()

	FSharedPoseHandle() = default;


	FSharedPoseHandle(FString InName)
		: PoseName(InName)
	{
	};

	UPROPERTY(EditAnywhere)
	FString PoseName;
};
