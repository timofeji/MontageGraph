// Copyright Timofej Jermolaev, All Rights Reserved.

#include "MontageGraphEditorModule.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_MontageGraph.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "FMontageGraphEditorModule"

void FMontageGraphEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();


	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked
		<FAssetToolsModule>("AssetTools").Get();

	EAssetTypeCategories::Type GameplayAssetCategory = AssetTools.
		RegisterAdvancedAssetCategory(
			FName(TEXT("Gameplay")),
			FText::FromName(TEXT("Gameplay")));

	TSharedPtr<IAssetTypeActions> MontageGraphType = MakeShareable(new
		FAssetTypeActions_MontageGraph(GameplayAssetCategory));

	AssetTools.RegisterAssetTypeActions(MontageGraphType.ToSharedRef());
}

void FMontageGraphEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMontageGraphEditorModule,
                 MontageGraphEditor)
