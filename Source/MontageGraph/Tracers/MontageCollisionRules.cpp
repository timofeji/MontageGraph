// Copyright Drop Games Inc.


#include "MontageCollisionRules.h"
// Called when the game starts
void UMontageCollisionRules::BeginPlay()
{
	Super::BeginPlay();
}

void UMontageCollisionRules::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);
	
}

void UMontageCollisionRules::ExecuteCollisionTraceTick(float MontageAlpha)
{
}
