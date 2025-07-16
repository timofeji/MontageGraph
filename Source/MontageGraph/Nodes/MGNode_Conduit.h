#pragma once

#include "CoreMinimal.h"
#include "MGNode.h"
#include "GameplayTagContainer.h"
#include "MGNode_Conduit.generated.h"


/**
 * Conduit node conducts graph connections based on gameplay tags on the owner character
 */
UCLASS(BlueprintType)
class MONTAGEGRAPH_API UMGNode_Conduit : public UMGNode
{
	GENERATED_BODY()
};
