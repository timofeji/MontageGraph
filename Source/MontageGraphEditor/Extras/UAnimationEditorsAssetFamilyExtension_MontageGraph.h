// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Editor/Persona/Public/AnimationEditorsAssetFamilyExtension.h"
#include "UAnimationEditorsAssetFamilyExtension_MontageGraph.generated.h"

UCLASS()
class UAnimationEditorsAssetFamilyExtension_MontageGraphAsset : public UAnimationEditorsAssetFamilyExtension
{
	GENERATED_BODY()

public:
	// UAnimationEditorsAssetFamilyExtension
	virtual TObjectPtr<UClass> GetAssetClass() const override;
	virtual FText GetAssetTypeDisplayName() const override;
	virtual const FSlateBrush* GetAssetTypeDisplayIcon() const override;
	virtual void FindAssetsOfType(TArray<FAssetData>& OutAssets, const IAnimationEditorsAssetFamilyInterface& AssetFamilyInterface) const override;
	virtual bool IsAssetCompatible(const FAssetData& InAssetData, const IAnimationEditorsAssetFamilyInterface& AssetFamilyInterface) const override;
	virtual void FindCounterpartAssets(const UObject* InAsset, IAnimationEditorsAssetFamilyInterface& AssetFamilyInterface) override;
	virtual void GetPosition(FName& OutBeforeClass, FName& OutAfterClass) const override;
};

