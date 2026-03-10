// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MGEdNode.h"
#include "MGEdNode_Edge.generated.h"

class UMontageGraph;
class UMGEdNode_Selector;
class UMGEdge;
class UMGEdNode;

UENUM(BlueprintType)
enum class ELinkTransitionRule : uint8
{
	Combo,
	TagQuery,
	GameplayEffectQuery,
	StackCount,
};


UCLASS(MinimalAPI)
class UMGEdNode_Edge : public UMGEdNode
{
	GENERATED_BODY()

public:
	UMGEdNode_Edge();
	virtual ~UMGEdNode_Edge();
	
	UPROPERTY(EditAnywhere, Category = "Transition Rules")
	ELinkTransitionRule  TransitionType;

	UPROPERTY(VisibleAnywhere, Instanced, Category = "Transition Rules")
	UMGEdge* RuntimeEdge;
	//
	// UPROPERTY(EditAnywhere, Category = "Transition Rules")
	// FGameplayTagQuery TransitionQuery;
	//
	// UPROPERTY(EditDefaultsOnly, Category = "Transition Rules")
	// FGameplayEffectQuery TransitionEffectQuery;

	UPROPERTY(EditAnywhere, Category = "Editor")
	FLinearColor EdgeColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);


	FLinearColor GetEdgeColor();

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	

	void SetRuntimeEdge(UMGEdge* InEdge);
	void CreateConnections(const UMGEdNode* Start, const UMGEdNode* End);
	// void CreateConnections(const UMGEdNodeBase* Start, const UMGEdNodeBase* End);
	void CreateBaseConnections(const UMGEdNode* Start, const UMGEdNode* End);

	UMGEdNode* GetStartNode() const;
	UMGEdNode* GetEndNode() const;
	


	FSlateColor GetEdgeColour() const { return EdgeColor; }

	//~ UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual void PrepareForCopying() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	//~ End UEdGraphNode interface

	void UpdateCachedIcon();
	UTexture2D* GetCachedIconTexture() const;
private:
	TSoftObjectPtr<UTexture2D> CachedIcon;

	void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event);
};
