// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphEdNode.h"
#include "HBMontageGraphEdNodeConduit.generated.h"



/** Editor Graph Node for conduit nodes in Montage Graphs. Based off UAnimStateConduitNode for state machine graphs in Anim BP. */
UCLASS(MinimalAPI)
class UMontageGraphEdNodeConduit : public UMontageGraphEdNode
{
	GENERATED_BODY()

public:
	UMontageGraphEdNodeConduit();

	//~ Begin UEdGraphNode Interface
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void    AllocateDefaultPins() override;
	virtual void    AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual FText   GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText   GetTooltipText() const override;
	virtual bool    CanDuplicateNode() const override { return false; }
	virtual void    ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	virtual FString GetDocumentationExcerptName() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void    NodeConnectionListChanged() override;
	//~ End UEdGraphNode Interface
};
