#pragma once

#include "CoreMinimal.h"
#include "MGNode.h"
#include "GameplayTagContainer.h"
#include "MGNode_Selector.generated.h"

/**
 * Selector node conducts graph connections based on gameplay tags on the owner character
 */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMGNode_Selector : public UMGNode
{
	GENERATED_BODY()
public:

	//Conducting state for each of the children nodes in order
	UPROPERTY(EditAnywhere, Category = "Selector")
	TArray<FGameplayTag> SelectorStates;
};
