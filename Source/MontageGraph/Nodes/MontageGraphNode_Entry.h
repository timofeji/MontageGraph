// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MGNode.h"
#include "GameplayTagContainer.h"
#include "MontageGraphNode_Entry.generated.h"

/** Runtime node for MontageGraph entry. */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMontageGraphNode_Entry : public UMGNode
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Entry")
	FGameplayTag EntryTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Network")
	FGameplayTag BitFlagTags;

	

	
#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
#endif
};

