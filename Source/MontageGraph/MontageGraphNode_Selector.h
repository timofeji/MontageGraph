#pragma once

#include "CoreMinimal.h"
#include "MontageGraphNode.h"
#include "GameplayTagContainer.h"
#include "MontageGraphNode_Selector.generated.h"

class UHBActionComponent;

/**
 * Selector node conducts graph connections based on gameplay tags on the owner character
 */
UCLASS(BlueprintType)
class MONTAGEGRAPH_API UMontageGraphNode_Selector : public UMontageGraphNode
{
	GENERATED_BODY()
public:

	//Conducting state for each of the children nodes in order
	UPROPERTY(EditAnywhere, Category = "Selector")
	TArray<FGameplayTag> ConductStates;
};
