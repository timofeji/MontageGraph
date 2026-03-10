#include "MontageGraphEditorModes.h" 

#include "AnimationEditorViewportClient.h"
#include "PersonaModule.h"
#include "IPersonaToolkit.h"
#include "IPersonaViewport.h"
#include "MontageGraphEditorToolbar.h"
#include "PersonaTabs.h"
#include "SBlueprintEditorToolbar.h"
#include "ToolMenus.h"

#include "MontageGraph/MontageGraph.h"


FMontageGraphEditorMode_Selection::FMontageGraphEditorMode_Selection(const TSharedRef<FMontageGraphEditor>& InMontageGraphEditor)
	: FApplicationMode(FMontageGraphEditorModes::Rules, FMontageGraphEditorModes::GetLocalizedMode)
{

	MGEditor = InMontageGraphEditor;
	

 	FPersonaViewportArgs ViewportArgs(InMontageGraphEditor->GetPersonaToolkit()->GetPreviewScene());
 	// ViewportArgs.BlueprintEditor = InMontageGraphEditor;
 	ViewportArgs.bShowStats = false;
	ViewportArgs.bShowPlaySpeedMenu = false;
	ViewportArgs.bShowTimeline = false;
	ViewportArgs.bShowTurnTable = false;
	ViewportArgs.bAlwaysShowTransformToolbar = true;
	// ViewportArgs.OnViewportCreated = FOnViewportCreated::CreateSP(&InMontageGraphEditor.Get(), &FMontageGraphEditorMode_Gameplay::HandleViewportCreated);
 
	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
 	TabFactories.RegisterFactory(PersonaModule.CreatePersonaViewportTabFactory(InMontageGraphEditor, ViewportArgs));
	TabFactories.RegisterFactory(PersonaModule.CreateAdvancedPreviewSceneTabFactory(InMontageGraphEditor, InMontageGraphEditor->GetPersonaToolkit()->GetPreviewScene()));
	//
	TabLayout = FTabManager::NewLayout("Standalone_MontageGraphMode_Gameplay_Layout_v1.5")
		->AddArea
		(
			// Main application area
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.2f)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->SetHideTabWell(true)
						->AddTab(FPersonaTabs::PreviewViewportID, ETabState::OpenedTab)
						->AddTab(FPersonaTabs::AdvancedPreviewSceneSettingsID, ETabState::ClosedTab)
					)
					->Split(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->AddTab(FPersonaTabs::AssetBrowserID, ETabState::OpenedTab)
					)
				)
				->Split
				(
					// Middle 
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.6f)
					->Split
					(
						// Middle top - document edit area
						FTabManager::NewStack()
						->SetSizeCoefficient(0.8f)
						->AddTab(FMontageGraphEditor::GraphViewportTabID,ETabState::OpenedTab)
					)
					 ->Split
					 (
					 	// Middle bottom - compiler results & find
					 	FTabManager::NewStack()
					 	->SetSizeCoefficient(0.2f)
					 	->AddTab(FMontageGraphEditor::AnimTimelineTabID, ETabState::OpenedTab)
					 )
				)
				->Split
				(
					// Right side
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						// Right top
						FTabManager::NewStack()
						->SetHideTabWell(false)
						->SetSizeCoefficient(1.f)
						// ->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
						// ->AddTab(FPersonaTabs::AdvancedPreviewSceneSettingsID, ETabState::OpenedTab)
						->AddTab(FMontageGraphEditor::DetailsTabID,ETabState::OpenedTab)
						
						// ->AddTab(FRigAnimAttributeTabSummoner::TabID, ETabState::OpenedTab)
						// ->SetForegroundTab(FBlueprintEditorTabs::DetailsID)
					)
				)
			)
		);
	//
	InMontageGraphEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
	InMontageGraphEditor->GetToolbarBuilder()->AddGameplayToolbar(ToolbarExtender);
}

void FMontageGraphEditorMode_Selection::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	const auto BP = MGEditor.Pin();
	BP->RegisterTabSpawners(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
}

void FMontageGraphEditorMode_Selection::PreDeactivateMode()
{
	FApplicationMode::PreDeactivateMode();

	check(MGEditor.IsValid());
	TSharedPtr<FMontageGraphEditor> MGEditorPtr  = MGEditor.Pin();
	
	// MGEditorPtr ->SaveEditedObjectState();
}

void FMontageGraphEditorMode_Selection::PostActivateMode()
{
	// Reopen any documents that were open when the blueprint was last saved
	check(MGEditor.IsValid());
	TSharedPtr<FMontageGraphEditor> MGEditorPtr = MGEditor.Pin();
	// MGEditorPtr ->RestoreBehaviorTree();

	FApplicationMode::PostActivateMode();
}




FMontageGraphEditorMode_Blends::FMontageGraphEditorMode_Blends(const TSharedRef<FMontageGraphEditor>& InMontageGraphEditor)
	: FApplicationMode(FMontageGraphEditorModes::Blends, FMontageGraphEditorModes::GetLocalizedMode)
{

	MGEditor = InMontageGraphEditor;



 	FPersonaViewportArgs ViewportArgs(InMontageGraphEditor->GetPersonaToolkit()->GetPreviewScene());
 	// ViewportArgs.BlueprintEditor = InMontageGraphEditor;
 	ViewportArgs.bShowStats = false;
	ViewportArgs.bShowPlaySpeedMenu = false;
	ViewportArgs.bShowTimeline = true;
	ViewportArgs.bShowTurnTable = false;
	ViewportArgs.bAlwaysShowTransformToolbar = true;
	
	// ViewportArgs.OnViewportCreated = FOnViewportCreated::CreateSP(&InMontageGraphEditor.Get(), &FMontageGraphEditorMode_Gameplay::HandleViewportCreated);
 
	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
 	TabFactories.RegisterFactory(PersonaModule.CreatePersonaViewportTabFactory(InMontageGraphEditor, ViewportArgs));
	TabFactories.RegisterFactory(PersonaModule.CreateAdvancedPreviewSceneTabFactory(InMontageGraphEditor, InMontageGraphEditor->GetPersonaToolkit()->GetPreviewScene()));
	//
	TabLayout = FTabManager::NewLayout("Standalone_MontageGraphMode_Blends_Layout_v1.5")
		->AddArea
		(
			// Main application area
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Horizontal)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.5f)
						->SetHideTabWell(true)
						->AddTab(FPersonaTabs::PreviewViewportID, ETabState::OpenedTab)
					
					)
				
				)
			
				
				->Split
				(
					// Middle 
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.6f)
					->Split
					(
						// Middle top - document edit area
						FTabManager::NewStack()
						->SetSizeCoefficient(0.8f)
						->AddTab(FMontageGraphEditor::BlendMatrixTabID,ETabState::OpenedTab)
					)
					// ->Split
					// (
					// 	// Middle bottom - compiler results & find
					// 	FTabManager::NewStack()
					// 	->SetSizeCoefficient(0.2f)
					// 	->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
					// 	->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
					// )
				)
				->Split
				(
					// Right side
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						// Right top
						FTabManager::NewStack()
						->SetHideTabWell(false)
						->SetSizeCoefficient(1.f)
						// ->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
						// ->AddTab(FPersonaTabs::AdvancedPreviewSceneSettingsID, ETabState::OpenedTab)
						->AddTab(FMontageGraphEditor::DetailsTabID,ETabState::OpenedTab)
						
						// ->AddTab(FRigAnimAttributeTabSummoner::TabID, ETabState::OpenedTab)
						// ->SetForegroundTab(FBlueprintEditorTabs::DetailsID)
					)
				)
			)
		);

	InMontageGraphEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
}

void FMontageGraphEditorMode_Blends::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	const auto BP = MGEditor.Pin();
	BP->RegisterTabSpawners(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
}

void FMontageGraphEditorMode_Blends::PostActivateMode()
{
	FApplicationMode::PostActivateMode();
	
	check(MGEditor.IsValid());
	TSharedPtr<FMontageGraphEditor> MGEditorPtr = MGEditor.Pin();
	MGEditorPtr->GetGraphEditorWidget()->NotifyGraphChanged();
}

FMontageGraphEditorMode_Debug::FMontageGraphEditorMode_Debug(const TSharedRef<FMontageGraphEditor>& InMontageGraphEditor)
	: FApplicationMode(FMontageGraphEditorModes::Debug, FMontageGraphEditorModes::GetLocalizedMode)
{

	MGEditor = InMontageGraphEditor;



 	FPersonaViewportArgs ViewportArgs(InMontageGraphEditor->GetPersonaToolkit()->GetPreviewScene());
 	// ViewportArgs.BlueprintEditor = InMontageGraphEditor;
 	ViewportArgs.bShowStats = false;
	ViewportArgs.bShowPlaySpeedMenu = false;
	ViewportArgs.bShowTimeline = true;
	ViewportArgs.bShowTurnTable = false;
	ViewportArgs.bAlwaysShowTransformToolbar = true;
	// ViewportArgs.OnViewportCreated = FOnViewportCreated::CreateSP(&InMontageGraphEditor.Get(), &FMontageGraphEditorMode_Gameplay::HandleViewportCreated);
 
	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
 	TabFactories.RegisterFactory(PersonaModule.CreatePersonaViewportTabFactory(InMontageGraphEditor, ViewportArgs));
	TabFactories.RegisterFactory(PersonaModule.CreateAdvancedPreviewSceneTabFactory(InMontageGraphEditor, InMontageGraphEditor->GetPersonaToolkit()->GetPreviewScene()));
	//
	TabLayout = FTabManager::NewLayout("Standalone_MontageGraphMode_Debug_Layout_v1.5")
		->AddArea
		(
			// Main application area
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Horizontal)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.5f)
						->SetHideTabWell(true)
						->AddTab(FPersonaTabs::PreviewViewportID, ETabState::OpenedTab)
					
					)
				
				)
			
				
				->Split
				(
					// Middle 
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.6f)
					->Split
					(
						// Middle top - document edit area
						FTabManager::NewStack()
						->SetSizeCoefficient(0.8f)
						->AddTab("Document", ETabState::OpenedTab)
					)
					// ->Split
					// (
					// 	// Middle bottom - compiler results & find
					// 	FTabManager::NewStack()
					// 	->SetSizeCoefficient(0.2f)
					// 	->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
					// 	->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
					// )
				)
				->Split
				(
					// Right side
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						// Right top
						FTabManager::NewStack()
						->SetHideTabWell(false)
						->SetSizeCoefficient(1.f)
						// ->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
						// ->AddTab(FPersonaTabs::AdvancedPreviewSceneSettingsID, ETabState::OpenedTab)
						->AddTab(FMontageGraphEditor::DetailsTabID,ETabState::OpenedTab)
						
						// ->AddTab(FRigAnimAttributeTabSummoner::TabID, ETabState::OpenedTab)
						// ->SetForegroundTab(FBlueprintEditorTabs::DetailsID)
					)
				)
			)
		);

	InMontageGraphEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
	InMontageGraphEditor->GetToolbarBuilder()->AddDebuggerToolbar(ToolbarExtender);
}

void FMontageGraphEditorMode_Debug::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	const auto BP = MGEditor.Pin();
	BP->RegisterTabSpawners(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
}
