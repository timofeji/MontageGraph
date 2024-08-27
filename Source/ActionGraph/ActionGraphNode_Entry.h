// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "ActionGraphNode.h"
#include "GameplayTagContainer.h"
#include "ActionGraphNode_Entry.generated.h"

/** Runtime node for combo graph entry. */
UCLASS(Blueprintable)
class ACTIONGRAPH_API UActionGraphNode_Entry : public UActionGraphNode
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entry")
	FGameplayTag EntryTag;

	int MaxDepth;
};
