// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MGEdge.generated.h"
class UMGNode;
struct FGameplayTag;
/**
 * 
 */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMGEdge : public UObject
{
	GENERATED_BODY()
public:
	
	/** Start or parent node for this edge */
	UPROPERTY(BlueprintReadOnly, Category = "MontageGraph")
	UMGNode* StartNode;

	/** End or child node for this edge */
	UPROPERTY(BlueprintReadOnly, Category = "MontageGraph")
	UMGNode* EndNode;

	
#if WITH_EDITORONLY_DATA
	FText NodeTitle;
#endif

#if WITH_EDITOR
	virtual FText GetNodeTitle() const { return NodeTitle; }
	virtual void  SetNodeTitle(const FText& InTitle);
#endif

};


UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMGEdge_TagQuery : public UMGEdge
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Transition Rules")
	FGameplayTagQuery Query;
};


UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMGEdge_GameplayEffectQuery : public UMGEdge
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Transition Rules")
	FGameplayEffectQuery Query;
};


UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMGEdge_StackCount : public UMGEdge
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Transition Rules")
	int32 RequiredNumOfStacks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Transition Rules")
	FGameplayEffectQuery Query;

	
};
