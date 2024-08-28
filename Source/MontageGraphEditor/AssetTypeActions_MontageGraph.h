// Copyright Drop Games Inc. 

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "MontageGraph/MontageGraph.h"

/**
 * 
 */
class FAssetTypeActions_MontageGraph : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_MontageGraph(EAssetTypeCategories::Type InAssetCategory)
		: AssetCategory(InAssetCategory){}


	/** Returns the color associated with this type */
	virtual FColor GetTypeColor() const override { return FColor(80,80,220); }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects,
	                             TSharedPtr<class IToolkitHost> EditWithinLevelEditor =
		                             TSharedPtr<
			                             IToolkitHost>()) override;

	// IAssetTypeActions Implementation
	virtual FText GetName() const override
	{
		return FText::FromName(TEXT("MontageGraph"));
	}

	virtual UClass* GetSupportedClass() const override
	{
		return UMontageGraph::StaticClass();
	}

	virtual uint32 GetCategories() override { return AssetCategory; }

private:
	EAssetTypeCategories::Type AssetCategory;
};
