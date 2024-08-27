// Created by Timofej Jermolaev, All rights reserved . 

#include "ActionGraphEditor.h"

#include "AnimationEditorPreviewActor.h"
#include "BlueprintEditor.h"
#include "EdGraphUtilities.h"

#include "EditorViewportCommands.h"
#include "GraphEditorActions.h"
#include "ActionGraphDebugger.h"
#include "ActionGraphEditorCommands.h"
#include "ActionGraphEditorMode.h"
#include "ActionGraphEditorStyles.h"
#include "ActionGraphEditorLog.h"
#include "PersonaModule.h"
#include "Framework/Commands/GenericCommands.h"
#include "Graph/HBActionGraphEdGraph.h"
#include "Graph/HBActionGraphSchema.h"
#include "Graph/Nodes/ActionGraphEdNode.h"
#include "Graph/Nodes/HBActionGraphEdNodeEdge.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "IPersonaToolkit.h"
#include "IPersonaPreviewScene.h"
#include "MaterialDomain.h"
#include "Animation/AnimMontage.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/ScopedSlowTask.h"

#include "ActionGraph/ActionGraph.h"

const FName FActionGraphEditor::ViewportTabID(
	TEXT("HBCharacterEditor_Viewport"));
const FName FActionGraphEditor::PaletteTabID(
	TEXT("HBCharacterEditor_ActionBinds"));
const FName FActionGraphEditor::DetailsTabID(TEXT("HBCharacterEditor_Details"));
const FName FActionGraphEditor::ActionNameTabID(TEXT("HBCharacterEditor_ActionName"));
const FName FActionGraphEditor::ActionGraphTabID(TEXT("HBCharacterEditor_ActionGraph"));

const FName FHBCharacterEditorModes::HBCombatEditorMode("CombatEditor");
const FName FHBCharacterEditorModes::HBThumbnailEditorMode("ThumbnailEditor");

const FName CharacterEditorAppName = FName(TEXT("HBCharacterEditor"));

#define LOCTEXT_NAMESPACE "FHBCharacterAssetEditor"

void FActionGraphEditor::OnSelectedNodesChanged(const TSet<UObject*>& Objects) const
{
}

void FActionGraphEditor::OnGraphActionMenuClosed(bool bArg, bool bCond) const
{
}

FActionMenuContent FActionGraphEditor::OnCreateGraphActionMenu(UEdGraph* EdGraph,
                                                                    const UE::Math::TVector2<double>& Vector2,
                                                                    const TArray<UEdGraphPin*>& EdGraphPins, bool bArg,
                                                                    TDelegate<void()> Delegate)
{
	return FActionMenuContent();
}


FActionGraphEditor::FActionGraphEditor()
{
}

FActionGraphEditor::~FActionGraphEditor()
{
}

void FActionGraphEditor::CreateInternalWidgets()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("CornerText", "Action Graph");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
		this, &FActionGraphEditor::OnGraphSelectionChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FActionGraphEditor::OnGraphNodeDoubleClicked);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FActionGraphEditor::OnNodeTitleCommitted);

	UEdGraph* EditorGraph = GraphBeingEdited->EditorGraph;
	check(EditorGraph);

	// Make full graph editor
	const bool bGraphIsEditable = EditorGraph->bEditable;
	GraphEditorView = SNew(SGraphEditor)
		.AdditionalCommands(DefaultCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditorGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(true);
}

void FActionGraphEditor::ExtendMenu()
{
}


void FActionGraphEditor::ExtendToolbar()
{
	// If the ToolbarExtender is valid, remove it before rebuilding it
	if (ToolbarExtender.IsValid())
	{
		RemoveToolbarExtender(ToolbarExtender);
		ToolbarExtender.Reset();
	}


	FName ParentName;
	static const FName MenuName = GetToolMenuToolbarName(ParentName);

	UToolMenu* ToolMenu = UToolMenus::Get()->ExtendMenu(MenuName);
	const FToolMenuInsert SectionInsertLocation("Asset", EToolMenuInsertType::After);

	const auto HBCharacterCommands = FActionGraphEditorCommands::Get();

	FToolMenuSection& HBCharacterSection = ToolMenu->AddSection("HBCharacter Commands",
	                                                            TAttribute<FText>(), SectionInsertLocation);

	FToolMenuEntry RegenerateTracersEntry = FToolMenuEntry::InitToolBarButton(
		HBCharacterCommands.RegenerateActionTracers,
		LOCTEXT("RegenerateActionTracers", ""),
		LOCTEXT("RegenerateActionTracersButtonTooltip", "Regenerates all Action Tracers for this character"),
		FSlateIcon(FActionGraphEditorStyles::Get().GetStyleSetName(), "HBEditor.Character.WeaponAction")
	);

	RegenerateTracersEntry.StyleNameOverride = FName("Toolbar.BackplateLeftPlay");

	HBCharacterSection.AddEntry(RegenerateTracersEntry);


	ToolMenu->AddDynamicSection("Persona", FNewToolMenuDelegate::CreateLambda([](UToolMenu* InToolMenu)
	{
		FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
		PersonaModule.AddCommonToolbarExtensions(InToolMenu);
	}), SectionInsertLocation);

	ToolbarExtender = MakeShareable(new FExtender);

	AddToolbarExtender(ToolbarExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
			{
				FillToolbar(ToolbarBuilder);


				// FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
				// TSharedRef<class IAssetFamily> AssetFamily = PersonaModule.CreatePersonaAssetFamily(
				// 	GraphBeingEdited->SkeletalMesh);
				// AddToolbarWidget(PersonaModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
			}
		));
}

void FActionGraphEditor::FillToolbar(FToolBarBuilder& ToolBarBuilder)
{
	// const UHBActionGraphEdGraph* EditorGraph = Cast<UHBActionGraphEdGraph>(
	// 	GraphBeingEdited->EditorGraph);
	// const TSharedRef<SWidget> SelectionBox = SNew(SComboButton)
	//    		.OnGetMenuContent(this, &FHBCharacterAssetEditor::OnGetDebuggerActorsMenu)
	//    		.ButtonContent()
	// [
	// 	SNew(STextBlock)
	//    			.ToolTipText(LOCTEXT("SelectDebugActor", "Pick actor to debug"))
	//    			.Text(EditorGraph && EditorGraph->Debugger.IsValid()
	//                       ? FText::FromString(EditorGraph->Debugger->GetDebuggedInstanceDesc())
	//                       : FText::GetEmpty())
	// ];
	//
	// ToolBarBuilder.BeginSection("World");
	// {
	// 	ToolBarBuilder.AddWidget(SelectionBox);
	// }
	// ToolBarBuilder.EndSection();
}


TSharedRef<SWidget> FActionGraphEditor::OnGetDebuggerActorsMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	const auto* EditorGraph = Cast<UHBActionGraphEdGraph>(GraphBeingEdited->EditorGraph);
	if (!EditorGraph)
	{
		return MenuBuilder.MakeWidget();
	}

	if (EditorGraph && EditorGraph->Debugger.IsValid())
	{
		TSharedPtr<FActionGraphDebugger> Debugger = EditorGraph->Debugger;

		TArray<AActor*> MatchingActors;
		TArray<UHBActionComponent*> MatchingInstances;
		Debugger->GetMatchingInstances(MatchingInstances, MatchingActors);

		// Fill the combo menu with actors that started the combo graph once
		for (int32 i = 0; i < MatchingActors.Num(); i++)
		{
			if (MatchingActors[i])
			{
				const FText ActorDesc = FText::FromString(Debugger->DescribeInstance(*MatchingActors[i]));
				TWeakObjectPtr<AActor> InstancePtr = MatchingActors[i];

				FUIAction ItemAction(FExecuteAction::CreateLambda([InstancePtr, EditorGraph]()
				{
					EditorGraph->Debugger->OnInstanceSelectedInDropdown(InstancePtr.Get());
				}));
				MenuBuilder.AddMenuEntry(ActorDesc, TAttribute<FText>(),
				                         FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage"),
				                         ItemAction);
			}
		}

		// Failsafe when no actor match
		if (MatchingActors.Num() == 0)
		{
			const FText ActorDesc = LOCTEXT("NoMatchForDebug", "Can't find matching actors");
			TWeakObjectPtr<AActor> InstancePtr;

			FUIAction ItemAction(FExecuteAction::CreateLambda([InstancePtr, EditorGraph]()
			{
				EditorGraph->Debugger->OnInstanceSelectedInDropdown(InstancePtr.Get());
			}));
			MenuBuilder.AddMenuEntry(ActorDesc, TAttribute<FText>(), FSlateIcon(), ItemAction);
		}
	}

	return MenuBuilder.MakeWidget();
}

bool FActionGraphEditor::IsPIESimulating() const
{
	return GEditor->bIsSimulatingInEditor || GEditor->PlayWorld;
}

void FActionGraphEditor::CreateDefaultCommands()
{
	if (DefaultCommands.IsValid())
	{
		return;
	}

	DefaultCommands = MakeShareable(new FUICommandList);

	// Common generic commands
	DefaultCommands->MapAction(
		FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FActionGraphEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FActionGraphEditor::CanSelectAllNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FActionGraphEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FActionGraphEditor::CanDeleteNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FActionGraphEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FActionGraphEditor::CanCopyNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FActionGraphEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FActionGraphEditor::CanCutNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FActionGraphEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FActionGraphEditor::CanPasteNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FActionGraphEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FActionGraphEditor::CanDuplicateNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FActionGraphEditor::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FActionGraphEditor::CanRenameNodes)
	);

	DefaultCommands->MapAction(
		FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateRaw(this, &FActionGraphEditor::OnCreateComment),
		FCanExecuteAction::CreateRaw(this, &FActionGraphEditor::CanCreateComment)
	);
}

void FActionGraphEditor::BindToolkitCommands()
{
	FActionGraphEditorCommands::Register();
	// Auto Arrange commands
	// ToolkitCommands->MapAction(
	// 	FHBActionGraphBlueprintEditorCommands::Get().AutoArrange,
	// 	FExecuteAction::CreateSP(this, &FHBCharacterAssetEditor::HandleAutoArrange),
	// 	FCanExecuteAction::CreateSP(this, &FHBCharacterAssetEditor::CanAutoArrange)
	// );
	//
	// ToolkitCommands->MapAction(
	// 	FHBActionGraphBlueprintEditorCommands::Get().AutoArrangeVertical,
	// 	FExecuteAction::CreateSP(this, &FHBCharacterAssetEditor::SetAndHandleAutoArrange, EHBActioGraphAutoArrangeStrategy::Vertical),
	// 	FCanExecuteAction::CreateSP(this, &FHBCharacterAssetEditor::CanAutoArrange)
	// );
	//

}

void FActionGraphEditor::InitCharacterEditor(EToolkitMode::Type Mode,
                                                  const TSharedPtr<IToolkitHost>&
                                                  InitToolkitHost,
                                                  UActionGraph* GraphToEdit)
{
	check(GraphToEdit);
	GraphBeingEdited = GraphToEdit;


	CreateDefaultCommands();
	BindToolkitCommands();


	FPersonaToolkitArgs PersonaToolkitArgs;
	PersonaToolkitArgs.OnPreviewSceneCreated =
		FOnPreviewSceneCreated::FDelegate::CreateSP(this, &FActionGraphEditor::HandlePreviewSceneCreated);

	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
	PersonaToolkit = PersonaModule.CreatePersonaToolkit(GraphBeingEdited, PersonaToolkitArgs);

	// PersonaToolkit->GetPreviewScene()->RegisterOnPreviewMeshChanged(
	// 	FOnPreviewMeshChanged::CreateSP(this, &FHBCharacterAssetEditor::HandlePreviewMeshChanged));

	// Set a default preview mesh, if any
	// PersonaToolkit->SetPreviewMesh(Character->SkeletalMesh, false);


	FEditorViewportCommands::Register();
	FGenericCommands::Register();
	FGraphEditorCommands::Register();


	CreateEditorGraph();
	CreateInternalWidgets();
	CreatePropertyWidget();


	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout =
		FTabManager::NewLayout("Standalone_HBCharacterEditor_Layout_v1")
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split(
				// Toolbar
				FTabManager::NewStack()
				->SetSizeCoefficient(0.2f)
				// #if NY_ENGINE_VERSION < 500
				// 				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
				// #endif
				->SetHideTabWell(true)
			)
			->Split(
				// Main Application area
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split(
					// Left
					// Details tab
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.2f)
					->Split(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(DetailsTabID,
						         ETabState::OpenedTab)
						->AddTab(ActionNameTabID,
						         ETabState::OpenedTab)
					)
					// ->SetOrientation(Orient_Vertical)
					// ->SetSizeCoefficient(0.2f)
					// ->Split(
					// 	FTabManager::NewStack()
					// 	->SetSizeCoefficient(0.07f)
					// )

				)

			)
		);


	// Initialize the asset editor and spawn nothing (dummy layout)
	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;
	constexpr bool bInIsToolbarFocusable = false;

	FAssetEditorToolkit::InitAssetEditor(Mode,
	                                     InitToolkitHost,
	                                     FName(TEXT("HBCharacterEditor")),
	                                     FTabManager::FLayout::NullLayout,
	                                     // StandaloneDefaultLayout,
	                                     bCreateDefaultStandaloneMenu,
	                                     bCreateDefaultToolbar,
	                                     GraphBeingEdited,
	                                     bInIsToolbarFocusable);


	UHBActionGraphEdGraph* HBActioGraphEd = Cast<UHBActionGraphEdGraph>(GraphBeingEdited->EditorGraph);
	check(HBActioGraphEd);

	HBActioGraphEd->Debugger = MakeShareable(new FActionGraphDebugger);
	HBActioGraphEd->Debugger->Setup(GraphBeingEdited, SharedThis(this));


	RebuildActionGraph();

	AddApplicationMode(
		FHBCharacterEditorModes::HBCombatEditorMode,
		MakeShareable(new FActionGraphEditorMode(SharedThis(this))));

	// AddApplicationMode(
	// 	FHBCharacterEditorModes::HBThumbnailEditorMode,
	// 	MakeShareable(new FHBThumbnailEditorMode(SharedThis(this))));

	SetCurrentMode(FHBCharacterEditorModes::HBCombatEditorMode);


	// extend menus and toolbar
	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

void FActionGraphEditor::HandlePreviewSceneCreated(
	const TSharedRef<IPersonaPreviewScene>& InPersonaPreviewScene)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	// load a ground mesh
	static const TCHAR* GroundAssetPath = TEXT("/Engine/MapTemplates/SM_Template_Map_Floor.SM_Template_Map_Floor");
	UStaticMesh* FloorMesh = Cast<UStaticMesh>(
		StaticLoadObject(UStaticMesh::StaticClass(), NULL, GroundAssetPath, NULL, LOAD_None, NULL));
	UMaterial* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
	check(FloorMesh);
	check(DefaultMaterial);

	// create ground mesh actor
	AStaticMeshActor* GroundActor = InPersonaPreviewScene->GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), FTransform::Identity);
	GroundActor->SetFlags(RF_Transient);
	GroundActor->GetStaticMeshComponent()->SetStaticMesh(FloorMesh);
	GroundActor->GetStaticMeshComponent()->SetMaterial(0, DefaultMaterial);
	GroundActor->SetMobility(EComponentMobility::Static);
	GroundActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GroundActor->GetStaticMeshComponent()->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	GroundActor->GetStaticMeshComponent()->bSelectable = false;
	// this will be an invisible collision box that users can use to test traces
	GroundActor->GetStaticMeshComponent()->SetVisibility(false);

	WeakGroundActorPtr = GroundActor;

	AAnimationEditorPreviewActor* Actor = InPersonaPreviewScene->GetWorld()->SpawnActor<AAnimationEditorPreviewActor>(
		AAnimationEditorPreviewActor::StaticClass(), FTransform::Identity);
	Actor->SetFlags(RF_Transient);
	InPersonaPreviewScene->SetActor(Actor);

	// Create the preview component
	UDebugSkelMeshComponent* EditorSkelComp = NewObject<UDebugSkelMeshComponent>(Actor);
	EditorSkelComp->SetSkeletalMesh(InPersonaPreviewScene->GetPersonaToolkit()->GetPreviewMesh());
	InPersonaPreviewScene->SetPreviewMeshComponent(EditorSkelComp);
	bool bWasCreated = false;
	// FAnimCustomInstanceHelper::BindToSkeletalMeshComponent<UControlRigLayerInstance>(EditorSkelComp, bWasCreated);
	InPersonaPreviewScene->AddComponent(EditorSkelComp, FTransform::Identity);

	// set root component, so we can attach to it. 
	Actor->SetRootComponent(EditorSkelComp);
	EditorSkelComp->bSelectable = false;
	EditorSkelComp->MarkRenderStateDirty();

	InPersonaPreviewScene->SetAllowMeshHitProxies(false);
	InPersonaPreviewScene->SetAdditionalMeshesSelectable(false);


	// if (GEditor)
	// {
	// 	// remove the preview scene undo handling - it has unwanted side effects
	// 	FAnimationEditorPreviewScene* AnimationEditorPreviewScene = static_cast<FAnimationEditorPreviewScene*>(&
	// 		InPersonaPreviewScene.Get());
	// 	if (AnimationEditorPreviewScene)
	// 	{
	// 		GEditor->UnregisterForUndo(AnimationEditorPreviewScene);
	// 	}
	// }
}

TSharedRef<SDockTab> FActionGraphEditor::SpawnTab_Details(const FSpawnTabArgs& SpawnTabArgs) const
{
	check(SpawnTabArgs.GetTabId() == DetailsTabID);

	// TODO use DialogueEditor.Tabs.Properties
	const auto* IconBrush = FAppStyle::GetBrush(TEXT("GenericEditor.Tabs.Properties"));

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
	.Label(LOCTEXT("CharacterDetailsTitle",
	               "Character Details"))
	.TabColorScale(GetTabColorScale())
	[

		DetailsView.ToSharedRef()
	];

	NewTab->SetTabIcon(IconBrush);

	return NewTab;
}

FEdGraphPinType FActionGraphEditor::GetTargetPinType() const
{
	// void* ValuePtr = nullptr;
	// if (TypePropertyHandle->GetValueData(ValuePtr) != FPropertyAccess::Fail)
	// {
	// 	return *((FEdGraphPinType*)ValuePtr);
	// }

	return FEdGraphPinType();
}

void FActionGraphEditor::HandlePinTypeChanged(const FEdGraphPinType& InPinType)
{
	void* ValuePtr = nullptr;
	if (TypePropertyHandle->GetValueData(ValuePtr) != FPropertyAccess::Fail)
	{
		TypePropertyHandle->NotifyPreChange();

		*((FEdGraphPinType*)ValuePtr) = InPinType;

		TypePropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		//
		// UAnimBlueprint* AnimBlueprint = WeakOuterNode->GetAnimBlueprint();
		// IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(
		// 	AnimBlueprint, false);
		// check(AssetEditor->GetEditorName() == "AnimationBlueprintEditor");

		LastGraphPinType = InPinType;
	}
}

void FActionGraphEditor::DebuggerUpdateGraph(bool bIsPIEActive)
{
	if (bIsPIEActive)
	{
		RebuildActionGraph();
	}
}


TSharedRef<SDockTab> FActionGraphEditor::SpawnTab_GraphViewport(const FSpawnTabArgs& SpawnTabArgs)
{
	check(SpawnTabArgs.GetTabId() == FActionGraphEditor::ActionGraphTabID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("ActionGraphViewportTab_Title", "Viewport"));

	if (GraphEditorView.IsValid())
	{
		SpawnedTab->SetContent(GraphEditorView.ToSharedRef());
	}

	return SpawnedTab;
}


void FActionGraphEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT(
		"WorkspaceMenu_HBCharacterEditor",
		"HBCharacter Editor"));
	TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.
		ToSharedRef();
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FActionGraphEditor::ActionGraphTabID,
	                                 FOnSpawnTab::CreateSP(this, &FActionGraphEditor::SpawnTab_GraphViewport))
	            .SetDisplayName(LOCTEXT("ActionGraphViewportTab", "GraphViewport"))
	            .SetGroup(WorkspaceMenuCategoryRef)
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));


	InTabManager->RegisterTabSpawner(FActionGraphEditor::DetailsTabID,
	                                 FOnSpawnTab::CreateSP(this, &FActionGraphEditor::SpawnTab_Details))
	            .SetDisplayName(LOCTEXT("HBActionGraphDetailsTab", "PropertyDetails"))
	            .SetGroup(WorkspaceMenuCategoryRef)
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FActionGraphEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

FText FActionGraphEditor::GetBaseToolkitName() const
{
	return LOCTEXT("HBCharacterEditorAppLabel", "HB Character Editor");
}

FText FActionGraphEditor::GetToolkitName() const
{
	const TArray<UObject*>& EditingObjs = GetEditingObjects();
	check(EditingObjs.Num() > 0);

	const UObject* EditingObject = EditingObjs[0];
	check(EditingObject);

	FFormatNamedArguments Args;
	Args.Add(TEXT("HBActionGraphName"), FText::FromString(EditingObject->GetName()));
	Args.Add(TEXT("DirtyState"), EditingObject->GetOutermost()->IsDirty()
		                             ? FText::FromString(TEXT("*"))
		                             : FText::GetEmpty());
	return FText::Format(LOCTEXT("HBActionGraphAssetEditorToolkitName", "{HBActionGraphName}{DirtyState}"), Args);
}

void FActionGraphEditor::RebuildActionGraph()
{
	if (!GraphBeingEdited || !GraphBeingEdited)
	{
		return;
	}

	if (auto Graph{CastChecked<UHBActionGraphEdGraph>(GraphBeingEdited->EditorGraph)})
	{
		Graph->RebuildGraph();
	}
}

void FActionGraphEditor::SaveAsset_Execute()
{
	if (GraphBeingEdited)
	{
		RebuildActionGraph();
	}

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FActionGraphEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	// if (GraphBeingEdited)
	// {
	// 	GraphBeingEdited->EditorGraph->GetSchema()->ForceVisualizationCacheClear();
	// }
}

void FActionGraphEditor::CreatePropertyWidget()
{
	FDetailsViewArgs Args;
	Args.bUpdatesFromSelection = false;
	Args.bLockable = false;
	Args.bAllowSearch = true;
	Args.NameAreaSettings = FDetailsViewArgs::ActorsUseNameArea;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = PropertyModule.CreateDetailView(Args);
	DetailsView->SetObject(GraphBeingEdited);
	DetailsView->OnFinishedChangingProperties().AddSP(
		this, &FActionGraphEditor::OnFinishedChangingProperties);
}

void FActionGraphEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Collector.AddReferencedObject(GraphBeingEdited->EditorGraph);
	// Collector.AddReferencedObject(GraphBeingEdited);
	Collector.AddReferencedObject(GraphBeingEdited);
}

void FActionGraphEditor::CreateEditorGraph()
{
	if (!GraphBeingEdited)
	{
		return;
	}

	if (!GraphBeingEdited)
	{
		GraphBeingEdited = NewObject<UActionGraph>(GraphBeingEdited,
		                                                              UActionGraph::StaticClass(),
		                                                              FName("ActionGraph"));
	}

	if (!GraphBeingEdited->EditorGraph)
	{
		// State Machine Graph
		GraphBeingEdited->EditorGraph = FBlueprintEditorUtils::CreateNewGraph(
			GraphBeingEdited,
			FName("ActionEdGraph"),
			UHBActionGraphEdGraph::StaticClass(),
			UHBActionGraphSchema::StaticClass()
		);
		GraphBeingEdited->EditorGraph->bAllowRenaming = false;
		GraphBeingEdited->EditorGraph->bAllowDeletion = false;

		// Give the schema a chance to fill out any required nodes (like the entry node)
		const UEdGraphSchema* GraphSchema = GraphBeingEdited->EditorGraph->GetSchema();
		GraphSchema->CreateDefaultNodesForGraph(*GraphBeingEdited->EditorGraph);
	}
}


FGraphPanelSelectionSet FActionGraphEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	if (GraphEditorView.IsValid())
	{
		CurrentSelection = GraphEditorView->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FActionGraphEditor::SelectAllNodes() const
{
	if (GraphEditorView.IsValid())
	{
		GraphEditorView->SelectAllNodes();
	}
}

bool FActionGraphEditor::CanSelectAllNodes()
{
	return true;
}

void FActionGraphEditor::DeleteSelectedNodes() const
{
	if (!GraphEditorView.IsValid())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	GraphEditorView->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GraphEditorView->GetSelectedNodes();
	GraphEditorView->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* EdNode = Cast<UEdGraphNode>(*NodeIt);
		if (!EdNode || !EdNode->CanUserDeleteNode())
		{
			continue;
		}

		if (UActionGraphEdNode* ActionGraphEdNode = Cast<UActionGraphEdNode>(EdNode))
		{
			ActionGraphEdNode->Modify();

			const UEdGraphSchema* Schema = ActionGraphEdNode->GetSchema();
			if (Schema)
			{
				Schema->BreakNodeLinks(*ActionGraphEdNode);
			}

			ActionGraphEdNode->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}
	}
}

bool FActionGraphEditor::CanDeleteNodes() const
{
	if (!GraphEditorView.IsValid())
	{
		return false;
	}

	if (IsPIESimulating())
	{
		return false;
	}

	const FGraphPanelSelectionSet SelectedNodes = GraphEditorView->GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FActionGraphEditor::DeleteSelectedDuplicateNodes() const
{
	if (!GraphEditorView.IsValid())
	{
		return;
	}

	const FGraphPanelSelectionSet OldSelectedNodes = GraphEditorView->GetSelectedNodes();
	GraphEditorView->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			GraphEditorView->SetNodeSelection(Node, true);
		}
	}

	// Delete the duplicate-able nodes
	DeleteSelectedNodes();

	GraphEditorView->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			GraphEditorView->SetNodeSelection(Node, true);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FActionGraphEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicateNodes();
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FActionGraphEditor::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FActionGraphEditor::CopySelectedNodes() const
{
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;
	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node == nullptr)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		if (const UActionGraphEdNodeEdge* ActionGraphEdNodeEdge = Cast<UActionGraphEdNodeEdge>(*SelectedIter))
		{
			const UActionGraphEdNode* StartNode = ActionGraphEdNodeEdge->GetStartNodeAsGraphNode();
			const UActionGraphEdNode* EndNode = ActionGraphEdNodeEdge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	// FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FActionGraphEditor::CanCopyNodes() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FActionGraphEditor::PasteNodes()
{
	if (GraphEditorView.IsValid())
	{
		PasteNodesHere(GraphEditorView->GetPasteLocation());
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FActionGraphEditor::PasteNodesHere(const FVector2D& Location)
{
	if (!GraphEditorView.IsValid())
	{
		return;
	}

	UEdGraph* EdGraph = GraphEditorView->GetCurrentGraph();

	{
		const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
		EdGraph->Modify();

		GraphEditorView->ClearSelectionSet();

		FString TextToImport;
		// FPlatformApplicationMisc::ClipboardPaste(TextToImport);

		TSet<UEdGraphNode*> PastedNodes;
		FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, PastedNodes);

		FVector2D AvgNodePosition(0.0f, 0.0f);

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			const UEdGraphNode* Node = *It;
			AvgNodePosition.X += Node->NodePosX;
			AvgNodePosition.Y += Node->NodePosY;
		}

		const float InvNumNodes = 1.0f / (PastedNodes.Num() == 0 ? 1 : PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;

		for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
		{
			UEdGraphNode* Node = *It;
			GraphEditorView->SetNodeSelection(Node, true);

			Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			Node->SnapToGrid(16);

			// Give new node a different Guid from the old one
			Node->CreateNewGuid();
		}
	}

	GraphEditorView->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		// ReSharper disable once CppExpressionWithoutSideEffects
		GraphOwner->MarkPackageDirty();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FActionGraphEditor::CanPasteNodes()
{
	if (!GraphEditorView.IsValid())
	{
		return false;
	}

	if (IsPIESimulating())
	{
		return false;
	}

	FString ClipboardContent;
	// FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(GraphEditorView->GetCurrentGraph(), ClipboardContent);
}

void FActionGraphEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FActionGraphEditor::CanDuplicateNodes()
{
	return CanCopyNodes();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FActionGraphEditor::OnRenameNode()
{
	if (GraphEditorView.IsValid())
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UEdGraphNode* SelectedNode = Cast<UEdGraphNode>(*NodeIt);
			if (SelectedNode && SelectedNode->bCanRenameNode)
			{
				GraphEditorView->IsNodeTitleVisible(SelectedNode, true);
				break;
			}
		}
	}
}

bool FActionGraphEditor::CanRenameNodes() const
{
	if (!GraphBeingEdited)
	{
		return false;
	}

	if (IsPIESimulating())
	{
		return false;
	}

	// UHBActionGraphEdGraph* HBActioGraphEd = Cast<UHBActionGraphEdGraph>(GraphBeingEdited->EditorGraph);
	// check(HBActioGraphEd);

	return GetSelectedNodes().Num() == 1;
}

bool FActionGraphEditor::CanCreateComment() const
{
	return GraphEditorView.IsValid() ? (GraphEditorView->GetNumberOfSelectedNodes() != 0) : false;
}

void FActionGraphEditor::OnCreateComment() const
{
	if (UEdGraph* EdGraph = GraphEditorView.IsValid() ? GraphEditorView->GetCurrentGraph() : nullptr)
	{
		const TSharedPtr<FEdGraphSchemaAction> Action = EdGraph->GetSchema()->GetCreateCommentAction();
		if (Action.IsValid())
		{
			Action->PerformAction(EdGraph, nullptr, FVector2D());
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FActionGraphEditor::OnGraphSelectionChanged(const TSet<UObject*>& NewSelection)
{
	ACTIONGRAPH_LOG(Verbose, TEXT("OnGraphSelectionChanged - %d"), NewSelection.Num())

	TArray<UObject*> SelectedNodes;
	TArray<UActionGraphEdNode*> GraphNodes;
	for (UObject* Selection : NewSelection)
	{
		SelectedNodes.Add(Selection);

		if (UActionGraphEdNode* Node = Cast<UActionGraphEdNode>(Selection))
		{
			GraphNodes.Add(Node);
		}
	}

	if (SelectedNodes.Num() > 0)
	{
		DetailsView->SetObjects(SelectedNodes);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FActionGraphEditor::OnGraphNodeDoubleClicked(UEdGraphNode* Node)
{
	ACTIONGRAPH_LOG(Verbose, TEXT("OnGraphNodeDoubleClicked - %s"), *GetNameSafe(Node))

	// TODO: Handle rename of node on double click
	// or opening of animation editor
}

void FActionGraphEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo,
                                                   UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		static const FText TransactionTitle = FText::FromString(FString(TEXT("Rename Node")));
		const FScopedTransaction Transaction(TransactionTitle);
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

#undef  LOCTEXT_NAMESPACE
