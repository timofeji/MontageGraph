#include "HBThumbnailEditorMode.h" 
#include "BlueprintEditorTabs.h"
#include "SBlueprintEditorToolbar.h"
#include "PersonaModule.h"
#include "IPersonaToolkit.h"
#include "PersonaTabs.h"
#include "ToolMenus.h"


#include "ActionGraph/ActionGraph.h"

FHBThumbnailEditorMode::FHBThumbnailEditorMode(const TSharedRef<FActionGraphEditor>& InHBCharacterEditor)
	: FApplicationMode(FHBCharacterEditorModes::HBThumbnailEditorMode)
{

	// HBCharacterBlueprintPtr = CastChecked<UHBCharacterBlueprint>(InHBCharacterEditor->GetBlueprintObj());

	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");

 	FPersonaViewportArgs ViewportArgs(InHBCharacterEditor->GetPersonaToolkit()->GetPreviewScene());
 	// ViewportArgs.BlueprintEditor = InHBCharacterEditor;
 	ViewportArgs.bShowStats = false;
	ViewportArgs.bShowPlaySpeedMenu = false;
	ViewportArgs.bShowTimeline = true;
	ViewportArgs.bShowTurnTable = false;
	ViewportArgs.bAlwaysShowTransformToolbar = true;
	// ViewportArgs.OnViewportCreated = FOnViewportCreated::CreateSP(InHBCharacterEditor, &FHBCharacterEditor::HandleViewportCreated);
 
 // 	TabFactories.RegisterFactory(PersonaModule.CreatePersonaViewportTabFactory(InHBCharacterEditor, ViewportArgs));
	// TabFactories.RegisterFactory(PersonaModule.CreateAdvancedPreviewSceneTabFactory(InHBCharacterEditor, InHBCharacterEditor->GetPersonaToolkit()->GetPreviewScene()));
	//
	// TabLayout = FTabManager::NewLayout("Standalone_HBCharacterEditMode_Layout_v1.5")
	// 	->AddArea
	// 	(
	// 		// Main application area
	// 		FTabManager::NewPrimaryArea()
	// 		->SetOrientation(Orient_Vertical)
	// 		->Split
	// 		(
	// 			FTabManager::NewSplitter()
	// 			->SetOrientation(Orient_Horizontal)
	// 			->Split
	// 			(
	// 				FTabManager::NewSplitter()
	// 				->SetOrientation(Orient_Vertical)
	// 				->SetSizeCoefficient(0.2f)
	// 				->Split
	// 				(
	// 					//	Left top - viewport
	// 					FTabManager::NewStack()
	// 					->SetSizeCoefficient(0.5f)
	// 					->SetHideTabWell(true)
	// 					->AddTab(FPersonaTabs::PreviewViewportID, ETabState::OpenedTab)
	// 				
	// 				)
	// 			
	// 			)
	// 			->Split
	// 			(
	// 				// Middle 
	// 				FTabManager::NewSplitter()
	// 				->SetOrientation(Orient_Vertical)
	// 				->SetSizeCoefficient(0.6f)
	// 				->Split
	// 				(
	// 					// Middle top - document edit area
	// 					FTabManager::NewStack()
	// 					->SetSizeCoefficient(0.8f)
	// 					->AddTab("Document", ETabState::ClosedTab)
	// 				)
	// 				->Split
	// 				(
	// 					// Middle bottom - compiler results & find
	// 					FTabManager::NewStack()
	// 					->SetSizeCoefficient(0.2f)
	// 					->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
	// 					->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
	// 				)
	// 			)
	// 			->Split
	// 			(
	// 				// Right side
	// 				FTabManager::NewSplitter()
	// 				->SetOrientation(Orient_Vertical)
	// 				->SetSizeCoefficient(0.2f)
	// 				->Split
	// 				(
	// 					// Right top
	// 					FTabManager::NewStack()
	// 					->SetHideTabWell(false)
	// 					->SetSizeCoefficient(1.f)
	// 					->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
	// 					->AddTab(FPersonaTabs::AdvancedPreviewSceneSettingsID, ETabState::OpenedTab)
	// 					// ->AddTab(FRigAnimAttributeTabSummoner::TabID, ETabState::OpenedTab)
	// 					->SetForegroundTab(FBlueprintEditorTabs::DetailsID)
	// 				)
	// 			)
	// 		)
	// 	);
	//
	// if (UToolMenu* Toolbar = InHBCharacterEditor->RegisterModeToolbarIfUnregistered(GetModeName()))
	// {
	// 	InHBCharacterEditor->GetToolbarBuilder()->AddCompileToolbar(Toolbar);
	// 	InHBCharacterEditor->GetToolbarBuilder()->AddScriptingToolbar(Toolbar);
	// 	InHBCharacterEditor->GetToolbarBuilder()->AddBlueprintGlobalOptionsToolbar(Toolbar);
	// }
}

void FHBThumbnailEditorMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	// TSharedPtr<FBlueprintEditor> BP = MyBlueprintEditor.Pin();
	//
	// BP->RegisterToolbarTab(InTabManager.ToSharedRef());
	//
	// // Mode-specific setup
	// BP->PushTabFactories(CoreTabFactories);
	// BP->PushTabFactories(BlueprintEditorTabFactories);
	// BP->PushTabFactories(TabFactories);
}
