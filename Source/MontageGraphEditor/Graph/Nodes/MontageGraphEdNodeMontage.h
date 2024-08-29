// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphEdNode.h"
#include "MontageGraphEdNodeMontage.generated.h"



/** Editor Graph Node for Selector nodes in Montage Graphs. Based off UAnimStateMontageNode for state machine graphs in Anim BP. */
UCLASS(MinimalAPI)
class UMontageGraphEdNodeMontage : public UMontageGraphEdNode
{
	GENERATED_BODY()

public:
	UMontageGraphEdNodeMontage();
	
	UPROPERTY(EditAnywhere, Category=Animation, meta=(DisplayPriority=-1))
	FString AnimationName;

	bool bHasLinkedAnimation = false;

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	virtual FString GetDocumentationExcerptName() const override;
	virtual FString GetDocumentationLink() const override;

	virtual void OnRenameNode(const FString& NewName) override;
	//~ End UEdGraphNode Interface
};
