#pragma once
#include "GameFramework/GameStateBase.h"
#include "Math/Vector.h"

namespace MontageGraphNetwork
{
	inline float GetServerTime(UObject* WorldContextObject)
	{
		if (!WorldContextObject)
			return 0.f;
		UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (!World)
			return 0.f;
		if (AGameStateBase* GameState = World->GetGameState())
		{
			return GameState->GetServerWorldTimeSeconds();
		}
		return World->GetTimeSeconds();
	}
}