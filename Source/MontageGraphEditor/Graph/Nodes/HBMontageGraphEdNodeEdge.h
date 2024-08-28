// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphEdNode.h"
#include "HBMontageGraphEdNodeEdge.generated.h"

class UMontageGraph;
class UMontageGraphEdNodeConduit;
class UMontageGraphEdge;
class UMontageGraphEdNode;

UCLASS(MinimalAPI)
class UMontageGraphEdNodeEdge : public UMontageGraphEdNode
{
	GENERATED_BODY()

public:
	UMontageGraphEdNodeEdge();
	virtual ~UMontageGraphEdNodeEdge();

	UPROPERTY(VisibleAnywhere, Instanced, Category = "HBMontageGraph")
	UMontageGraphEdge* RuntimeEdge;

	void SetRuntimeEdge(UMontageGraphEdge* InEdge);
	void CreateConnections(const UMontageGraphEdNode* Start, const UMontageGraphEdNode* End);
	// void CreateConnections(const UMontageGraphEdNodeBase* Start, const UMontageGraphEdNodeBase* End);
	void CreateBaseConnections(const UMontageGraphEdNode* Start, const UMontageGraphEdNode* End);

	UMontageGraphEdNode* GetStartNodeAsBase() const;
	UMontageGraphEdNode* GetStartNodeAsGraphNode() const;
	UMontageGraphEdNodeConduit* GetStartNodeAsConduit() const;
	UMontageGraphEdNode* GetEndNode() const;

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
