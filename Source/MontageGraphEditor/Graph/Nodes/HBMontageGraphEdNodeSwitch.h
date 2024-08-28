// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphEdNode.h"
#include "UObject/Object.h"
#include "HBMontageGraphEdNodeSwitch.generated.h"

/**
 * 
 */
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageGraphEdNodeSwitch : public UMontageGraphEdNode
{
	GENERATED_BODY()
	
public:
	UMontageGraphEdNodeSwitch();

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	virtual FString GetDocumentationExcerptName() const override;
	virtual FString GetDocumentationLink() const override;
	//~ End UEdGraphNode Interface
};
