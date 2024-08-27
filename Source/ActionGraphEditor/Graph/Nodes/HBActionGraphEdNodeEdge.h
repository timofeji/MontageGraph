// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "ActionGraphEdNode.h"
#include "HBActionGraphEdNodeEdge.generated.h"

class UActionGraph;
class UActionGraphEdNodeConduit;
class UActionGraphEdge;
class UActionGraphEdNode;

UCLASS(MinimalAPI)
class UActionGraphEdNodeEdge : public UActionGraphEdNode
{
	GENERATED_BODY()

public:
	UActionGraphEdNodeEdge();
	virtual ~UActionGraphEdNodeEdge();

	UPROPERTY(VisibleAnywhere, Instanced, Category = "HBActionGraph")
	UActionGraphEdge* RuntimeEdge;

	void SetRuntimeEdge(UActionGraphEdge* InEdge);
	void CreateConnections(const UActionGraphEdNode* Start, const UActionGraphEdNode* End);
	// void CreateConnections(const UActionGraphEdNodeBase* Start, const UActionGraphEdNodeBase* End);
	void CreateBaseConnections(const UActionGraphEdNode* Start, const UActionGraphEdNode* End);

	UActionGraphEdNode* GetStartNodeAsBase() const;
	UActionGraphEdNode* GetStartNodeAsGraphNode() const;
	UActionGraphEdNodeConduit* GetStartNodeAsConduit() const;
	UActionGraphEdNode* GetEndNode() const;

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
