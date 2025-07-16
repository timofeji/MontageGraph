// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphModule.h"
#include "UObject/Object.h"
#include "MontageGraphGlobals.generated.h"

struct FGameplayTag;
class UMontageGraph;
class UCollisionTracer;


/**
 * 
 */
UCLASS()
class MONTAGEGRAPH_API UMontageGraphGlobals : public UObject
{
	GENERATED_BODY()

	
public:
	
	/** Gets the single instance of the globals object, will create it as necessary */
	static UMontageGraphGlobals & Get()
	{
		return *IMontageGraphModule::Get().GetMontageGraphGlobals();
	}
	
	virtual void InitGlobalData();
	void ResetCachedData();
	
	
	//Traverses the graph using BFS and loads/caches the necessary montages/data
	UFUNCTION(BlueprintCallable)
	void LoadGraphBindBranch(const FGameplayTag& BindTag);
	
	const UMontageGraph* GetMontageGraphForActor(AActor* InActor);
};


