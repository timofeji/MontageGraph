// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "MontageCollisionRules.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MONTAGEGRAPH_API UMontageCollisionRules : public UActorComponent
{
	GENERATED_BODY()


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void RegisterComponentTickFunctions(bool bRegister) override;

	virtual void ExecuteCollisionTraceTick(float MontageAlpha);

};
