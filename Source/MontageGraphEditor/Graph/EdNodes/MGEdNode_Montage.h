// Created by Timofej Jermolaev, All rights reserved . 

#pragma once



#include "CoreMinimal.h"
#include "MGEdNode.h"
#include "MGEdNode_Montage.generated.h"

class UDopeSheetTrackBase;
class FDopeSheetTrackModel;

/** Editor Graph Node for Selector nodes in Montage Graphs. Based off UAnimStateMontageNode for state machine graphs in Anim BP. */
UCLASS(MinimalAPI)
class UMGEdNode_Montage : public UMGEdNode
{
	GENERATED_BODY()

public:
	UMGEdNode_Montage();
	
	UPROPERTY(EditAnywhere, Category = "Settings| Default Blend")
	FMontageBlendSettings BlendInSettings;
	
	UPROPERTY(EditAnywhere, Category = "Settings| Default Blend")
	FMontageBlendSettings BlendOutSettings;
	
	UPROPERTY(EditAnywhere, Category = "Settings| Default Blend")
	float InBlendOutTriggerTime = -1.f;
	
	UPROPERTY()
	TArray<UDopeSheetTrackBase*> MontageTracks;
	
	UPROPERTY(AssetRegistrySearchable)
	FString MontageDisplayName;
	//
	// UPROPERTY(EditAnywhere, Category = "Settings")
	FName SlotName;


	void MarkStale();
	void RegenerateMontage(class UMontageGraph* Graph);
	bool bShouldRegenerate;


	//~ Begin UEdGraphNode Interface
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
	virtual void OnRenameNode(const FString& NewName) override;
	virtual void PostPlacedNewNode() override;
	FLinearColor GetNodeTitleColor() const override;
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface
	
	UAnimMontage* GetMontage();
	
	static const FName DefaultSlotName;

};
