// Copyright Drop Games Inc. 

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "ActionGraph/ActionGraph.h"

/**
 * 
 */
class FAssetTypeActions_ActionGraph : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_ActionGraph(EAssetTypeCategories::Type InAssetCategory)
		: AssetCategory(InAssetCategory){}


	/** Returns the color associated with this type */
	virtual FColor GetTypeColor() const override { return FColor(80,123,72); }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects,
	                             TSharedPtr<class IToolkitHost> EditWithinLevelEditor =
		                             TSharedPtr<
			                             IToolkitHost>()) override;

	// IAssetTypeActions Implementation
	virtual FText GetName() const override
	{
		return FText::FromName(TEXT("ActionGraph"));
	}

	virtual UClass* GetSupportedClass() const override
	{
		return UActionGraph::StaticClass();
	}

	virtual uint32 GetCategories() override { return AssetCategory; }

private:
	EAssetTypeCategories::Type AssetCategory;
};
