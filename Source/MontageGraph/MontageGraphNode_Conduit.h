#pragma once

#include "CoreMinimal.h"
#include "MontageGraphNode.h"
#include "GameplayTagContainer.h"
#include "MontageGraphNode_Conduit.generated.h"

class UHBActionComponent;

/**
 * Conduit node conducts graph connections based on gameplay tags on the owner character
 */
UCLASS(BlueprintType)
class MONTAGEGRAPH_API UMontageGraphNode_Conduit : public UMontageGraphNode
{
	GENERATED_BODY()
public:

	//Conducting state for each of the children nodes in order
	UPROPERTY(EditAnywhere, Category = "Conduit")
	TArray<FGameplayTag> ConductStates;
};
