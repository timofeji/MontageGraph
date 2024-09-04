#include "SharedPosesEngineSubsystem.h"

// TArray<TSharedPtr<FSharedPoseHandle>> USharedPosesEngineSubsystem::GetSharedPoseData()
// {
// 	return nullptr;
// }

void USharedPosesEngineSubsystem::AddNewHandle(FSharedPoseHandle Shared)
{
	SharedPoseData.Add(Shared);
}
