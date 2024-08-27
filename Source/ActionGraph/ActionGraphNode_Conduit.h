#pragma once

#include "CoreMinimal.h"
#include "ActionGraphNode.h"
#include "GameplayTagContainer.h"
#include "ActionGraphNode_Conduit.generated.h"

class UHBActionComponent;

/**
 * Conduit node conducts graph connections based on gameplay tags on the owner character
 */
UCLASS(BlueprintType)
class ACTIONGRAPH_API UActionGraphNode_Conduit : public UActionGraphNode
{
	GENERATED_BODY()
public:

	//Conducting state for each of the children nodes in order
	UPROPERTY(EditAnywhere, Category = "Conduit")
	TArray<FGameplayTag> ConductStates;
};
