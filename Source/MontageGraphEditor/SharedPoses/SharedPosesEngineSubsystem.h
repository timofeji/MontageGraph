#pragma once
#include "Subsystems/EngineSubsystem.h"

#include "SharedPosesEngineSubsystem.generated.h"

UCLASS()
class MONTAGEGRAPHEDITOR_API USharedPosesEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Content")
	TArray<FSharedPoseHandle> SharedPoseData;

	void AddNewHandle(FSharedPoseHandle NewHandle);
	
	// TArray<TSharedPtr<FSharedPoseHandle>> GetSharedPoseData();
	
#endif

};
