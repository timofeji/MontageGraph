// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphNode.h"
#include "GameplayTagContainer.h"
#include "MontageGraphNode_Entry.generated.h"

/** Runtime node for combo graph entry. */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMontageGraphNode_Entry : public UMontageGraphNode
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entry")
	FGameplayTag EntryTag;

	int MaxDepth;
};
