// Copyright Timofej Jermolaev, All Rights Reserved.

#include "MontageGraphEditorModule.h"

#include "AssetToolsModule.h"
#include "EdGraphUtilities.h"
#include "IAssetTools.h"

#include "AssetTypeActions_MontageGraph.h"
#include "MontageGraphEditorStyle.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "GameFramework/HUD.h"

#include "Graph/MontageGraphNodePanelFactory.h"
#include "Graph/EdNodes/MGEdNode.h"
#include "Graph/EdNodes/MGEdNode_Montage.h"
#include "MontageGraph/MontageGraphComponent.h"
#include "Slate/MontageGraphDetails.h"
#include "Tracks/MontageTrack_BlendLinks.h"

#define LOCTEXT_NAMESPACE "FMontageGraphEditorModule"

static void RefreshSlateStyles()
{
	FMontageGraphEditorStyle::ReloadTextures();
	UE_LOG(LogTemp, Log, TEXT("Slate styles and textures refreshed."));
}

// Register the console command
FAutoConsoleCommand RefreshStyleCommand(
	TEXT("MontageGraphEditor.RefreshStyles"),
	TEXT("Refreshes all registered Slate styles and forces textures to update."),
	FConsoleCommandDelegate::CreateStatic(RefreshSlateStyles)
);

void FMontageGraphEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();

	FMontageGraphEditorStyle::Initialize();
	FMontageGraphEditorStyle::ReloadTextures();

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

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(UMGEdNode_Montage::StaticClass()->GetFName(),
	                                               FOnGetDetailCustomizationInstance::CreateStatic(
		                                               &FMontageGraphDetails::MakeInstance));
	
	PropertyEditorModule.RegisterCustomClassLayout(UMontageTrackSection_LinkBlend::StaticClass()->GetFName(),
	                                               FOnGetDetailCustomizationInstance::CreateStatic(
		                                               &UMontageTrackSection_LinkBlendDetails::MakeInstance));

	
	if (!IsRunningDedicatedServer())
	{
		AHUD::OnShowDebugInfo.AddStatic(&UMontageGraphComponent::OnShowDebugInfo);
	}

}

void FMontageGraphEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();

	FMontageGraphEditorStyle::Shutdown();
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(
		"PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout("MGEdNode");

	FEdGraphUtilities::UnregisterVisualNodeFactory(GraphNodeFactory);

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMontageGraphEditorModule, MontageGraphEditor)
