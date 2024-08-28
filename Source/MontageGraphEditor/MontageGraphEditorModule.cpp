// Copyright Timofej Jermolaev, All Rights Reserved.

#include "MontageGraphEditorModule.h"

#include "AssetToolsModule.h"
#include "EdGraphUtilities.h"
#include "IAssetTools.h"

#include "AssetTypeActions_MontageGraph.h"
#include "Graph/MontageGraphNodePanelFactory.h"

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

	
	GraphNodeFactory = MakeShareable(new FMontageGraphNodePanelFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphNodeFactory);
}

void FMontageGraphEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMontageGraphEditorModule,MontageGraphEditor)

