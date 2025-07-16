// Copyright Drop Games Inc.


#include "MontageGraphGlobals.h"
#include "MontageGraph.h"
#include "MontageGraphComponent.h"


void UMontageGraphGlobals::InitGlobalData()
{
}

void UMontageGraphGlobals::ResetCachedData()
{

}

void UMontageGraphGlobals::LoadGraphBindBranch(const FGameplayTag& BindTag)
{
}

const UMontageGraph* UMontageGraphGlobals::GetMontageGraphForActor(AActor* InActor)
{
	UMontageGraphComponent* MontageGraphComponent = InActor->FindComponentByClass<UMontageGraphComponent>();
	if (MontageGraphComponent)
	{
		return MontageGraphComponent->GetGraph();
	}

	return nullptr;	
}

