// Copyright Timofej Jermolaev, All Rights Reserved.

#include "ActionGraphEditorModule.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_ActionGraph.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "FActionGraphEditorModule"

void FActionGraphEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();


	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked
		<FAssetToolsModule>("AssetTools").Get();

	EAssetTypeCategories::Type GameplayAssetCategory = AssetTools.
		RegisterAdvancedAssetCategory(
			FName(TEXT("Gameplay")),
			FText::FromName(TEXT("Gameplay")));

	TSharedPtr<IAssetTypeActions> ActionGraphType = MakeShareable(new
		FAssetTypeActions_ActionGraph(GameplayAssetCategory));

	AssetTools.RegisterAssetTypeActions(ActionGraphType.ToSharedRef());
}

void FActionGraphEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActionGraphEditorModule,
                 ActionGraphEditor)
