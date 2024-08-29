// Created by Timofej Jermolaev, All rights reserved . 

#include "MontageGraphEditor.h"

#include "AnimationEditorPreviewActor.h"
#include "BlueprintEditor.h"
#include "EdGraphUtilities.h"

#include "EditorViewportCommands.h"
#include "GraphEditorActions.h"
#include "MontageGraphDebugger.h"
#include "MontageGraphEditorCommands.h"
#include "MontageGraphEditorMode.h"
#include "MontageGraphEditorStyles.h"
#include "MontageGraphEditorLog.h"
#include "PersonaModule.h"
#include "Framework/Commands/GenericCommands.h"
#include "Graph/MontageGraphEdGraph.h"
#include "Graph/MontageGraphSchema.h"
#include "Graph/Nodes/MontageGraphEdNode.h"
#include "Graph/Nodes/HBMontageGraphEdNodeEdge.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "IPersonaToolkit.h"
#include "IPersonaPreviewScene.h"
#include "MaterialDomain.h"
#include "Animation/AnimMontage.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/ScopedSlowTask.h"

#include "MontageGraph/MontageGraph.h"

const FName FMontageGraphEditor::DetailsTabID(TEXT("MontageGraph_Details"));
const FName FMontageGraphEditor::GraphViewportTabID(TEXT("MontageGraph_Viewport"));

const FName FMontageGraphEditorModes::MontageGraphEditorMode("MontageGraphEditor");

const FName CharacterEditorAppName = FName(TEXT("HBCharacterEditor"));

#define LOCTEXT_NAMESPACE "FHBCharacterAssetEditor"

void FMontageGraphEditor::OnSelectedNodesChanged(const TSet<UObject*>& Objects) const
{
}

void FMontageGraphEditor::OnGraphActionMenuClosed(bool bArg, bool bCond) const
{
}

FActionMenuContent FMontageGraphEditor::OnCreateGraphActionMenu(UEdGraph* EdGraph,
                                                                    const UE::Math::TVector2<double>& Vector2,
                                                                    const TArray<UEdGraphPin*>& EdGraphPins, bool bArg,
                                                                    TDelegate<void()> Delegate)
{
	return FActionMenuContent();
}


FMontageGraphEditor::FMontageGraphEditor()
{
}

FMontageGraphEditor::~FMontageGraphEditor()
{
}

void FMontageGraphEditor::CreateInternalWidgets()
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("CornerText", "Montage Graph");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
		this, &FMontageGraphEditor::OnGraphSelectionChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FMontageGraphEditor::OnGraphNodeDoubleClicked);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FMontageGraphEditor::OnNodeTitleCommitted);

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

void FMontageGraphEditor::ExtendMenu()
{
}


void FMontageGraphEditor::ExtendToolbar()
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

	const auto HBCharacterCommands = FMontageGraphEditorCommands::Get();

	FToolMenuSection& MontageGraphTabSection = ToolMenu->AddSection("HBCharacter Commands",
	                                                            TAttribute<FText>(), SectionInsertLocation);

	FToolMenuEntry RegenerateTracersEntry = FToolMenuEntry::InitToolBarButton(
		HBCharacterCommands.EditClassDefaults,
		LOCTEXT("Edit Class Defaults", ""),
		LOCTEXT("EditClassDefaultsTooltip", "Edit default values of this Montage Graph"),
		FSlateIcon(FMontageGraphEditorStyles::Get().GetStyleSetName(), "HBEditor.Character.WeaponAction")
	);
	
	RegenerateTracersEntry.StyleNameOverride = FName("Toolbar.BackplateLeftPlay");
	
	MontageGraphTabSection.AddEntry(RegenerateTracersEntry);


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

void FMontageGraphEditor::FillToolbar(FToolBarBuilder& ToolBarBuilder)
{
	// const UMontageGraphEdGraph* EditorGraph = Cast<UMontageGraphEdGraph>(
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


TSharedRef<SWidget> FMontageGraphEditor::OnGetDebuggerActorsMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	const auto* EditorGraph = Cast<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph);
	if (!EditorGraph)
	{
		return MenuBuilder.MakeWidget();
	}

	if (EditorGraph && EditorGraph->Debugger.IsValid())
	{
		TSharedPtr<FMontageGraphDebugger> Debugger = EditorGraph->Debugger;

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

bool FMontageGraphEditor::IsPIESimulating() const
{
	return GEditor->bIsSimulatingInEditor || GEditor->PlayWorld;
}

void FMontageGraphEditor::CreateDefaultCommands()
{
	if (DefaultCommands.IsValid())
	{
		return;
	}

	DefaultCommands = MakeShareable(new FUICommandList);

	// Common generic commands
	DefaultCommands->MapAction(
		FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FMontageGraphEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FMontageGraphEditor::CanSelectAllNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FMontageGraphEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMontageGraphEditor::CanDeleteNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FMontageGraphEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMontageGraphEditor::CanCopyNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FMontageGraphEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FMontageGraphEditor::CanCutNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FMontageGraphEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FMontageGraphEditor::CanPasteNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FMontageGraphEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FMontageGraphEditor::CanDuplicateNodes)
	);

	DefaultCommands->MapAction(
		FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FMontageGraphEditor::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FMontageGraphEditor::CanRenameNodes)
	);

	DefaultCommands->MapAction(
		FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateRaw(this, &FMontageGraphEditor::OnCreateComment),
		FCanExecuteAction::CreateRaw(this, &FMontageGraphEditor::CanCreateComment)
	);
}

void FMontageGraphEditor::BindToolkitCommands()
{
	FMontageGraphEditorCommands::Register();
	// Auto Arrange commands
	// ToolkitCommands->MapAction(
	// 	FHBMontageGraphBlueprintEditorCommands::Get().AutoArrange,
	// 	FExecuteAction::CreateSP(this, &FMontageGraphEditor::HandleAutoArrange),
	// 	FCanExecuteAction::CreateSP(this, &FMontageGraphEditor::CanAutoArrange)
	// );
	//
	// ToolkitCommands->MapAction(
	// 	FHBMontageGraphBlueprintEditorCommands::Get().AutoArrangeVertical,
	// 	FExecuteAction::CreateSP(this, &FMontageGraphEditor::SetAndHandleAutoArrange, EHBActioGraphAutoArrangeStrategy::Vertical),
	// 	FCanExecuteAction::CreateSP(this, &FMontageGraphEditor::CanAutoArrange)
	// );
	

}

void FMontageGraphEditor::InitCharacterEditor(EToolkitMode::Type Mode,
                                                  const TSharedPtr<IToolkitHost>&
                                                  InitToolkitHost,
                                                  UMontageGraph* GraphToEdit)
{
	check(GraphToEdit);
	GraphBeingEdited = GraphToEdit;


	CreateDefaultCommands();
	BindToolkitCommands();


	FPersonaToolkitArgs PersonaToolkitArgs;
	PersonaToolkitArgs.OnPreviewSceneCreated =
		FOnPreviewSceneCreated::FDelegate::CreateSP(this, &FMontageGraphEditor::HandlePreviewSceneCreated);

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


	UMontageGraphEdGraph* HBActioGraphEd = Cast<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph);
	check(HBActioGraphEd);

	HBActioGraphEd->Debugger = MakeShareable(new FMontageGraphDebugger);
	HBActioGraphEd->Debugger->Setup(GraphBeingEdited, SharedThis(this));


	RebuildMontageGraph();

	AddApplicationMode(
		FMontageGraphEditorModes::MontageGraphEditorMode,
		MakeShareable(new FMontageGraphEditorMode(SharedThis(this))));

	SetCurrentMode(FMontageGraphEditorModes::MontageGraphEditorMode);


	// extend menus and toolbar
	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

void FMontageGraphEditor::HandlePreviewSceneCreated(
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

TSharedRef<SDockTab> FMontageGraphEditor::SpawnTab_Details(const FSpawnTabArgs& SpawnTabArgs) const
{
	check(SpawnTabArgs.GetTabId() == DetailsTabID);

	// TODO use DialogueEditor.Tabs.Properties
	const auto* IconBrush = FAppStyle::GetBrush(TEXT("GenericEditor.Tabs.Properties"));

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
	.Label(LOCTEXT("MontageGraphDetailsTitle",
	               "Details"))
	.TabColorScale(GetTabColorScale())
	[

		DetailsView.ToSharedRef()
	];

	NewTab->SetTabIcon(IconBrush);

	return NewTab;
}

FEdGraphPinType FMontageGraphEditor::GetTargetPinType() const
{
	// void* ValuePtr = nullptr;
	// if (TypePropertyHandle->GetValueData(ValuePtr) != FPropertyAccess::Fail)
	// {
	// 	return *((FEdGraphPinType*)ValuePtr);
	// }

	return FEdGraphPinType();
}

void FMontageGraphEditor::HandlePinTypeChanged(const FEdGraphPinType& InPinType)
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

void FMontageGraphEditor::DebuggerUpdateGraph(bool bIsPIEActive)
{
	if (bIsPIEActive)
	{
		RebuildMontageGraph();
	}
}


TSharedRef<SDockTab> FMontageGraphEditor::SpawnTab_GraphViewport(const FSpawnTabArgs& SpawnTabArgs)
{
	check(SpawnTabArgs.GetTabId() == FMontageGraphEditor::GraphViewportTabID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("MontageGraphViewportTab_Title", "Viewport"));

	if (GraphEditorView.IsValid())
	{
		SpawnedTab->SetContent(GraphEditorView.ToSharedRef());
	}

	return SpawnedTab;
}


void FMontageGraphEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT(
		"WorkspaceMenu_HBCharacterEditor",
		"HBCharacter Editor"));
	TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.
		ToSharedRef();
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FMontageGraphEditor::GraphViewportTabID,
	                                 FOnSpawnTab::CreateSP(this, &FMontageGraphEditor::SpawnTab_GraphViewport))
	            .SetDisplayName(LOCTEXT("MontageGraphViewportTab", "GraphViewport"))
	            .SetGroup(WorkspaceMenuCategoryRef)
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));


	InTabManager->RegisterTabSpawner(FMontageGraphEditor::DetailsTabID,
	                                 FOnSpawnTab::CreateSP(this, &FMontageGraphEditor::SpawnTab_Details))
	            .SetDisplayName(LOCTEXT("HBMontageGraphDetailsTab", "PropertyDetails"))
	            .SetGroup(WorkspaceMenuCategoryRef)
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FMontageGraphEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

FText FMontageGraphEditor::GetBaseToolkitName() const
{
	return LOCTEXT("HBCharacterEditorAppLabel", "HB Character Editor");
}

FText FMontageGraphEditor::GetToolkitName() const
{
	const TArray<UObject*>& EditingObjs = GetEditingObjects();
	check(EditingObjs.Num() > 0);

	const UObject* EditingObject = EditingObjs[0];
	check(EditingObject);

	FFormatNamedArguments Args;
	Args.Add(TEXT("HBMontageGraphName"), FText::FromString(EditingObject->GetName()));
	Args.Add(TEXT("DirtyState"), EditingObject->GetOutermost()->IsDirty()
		                             ? FText::FromString(TEXT("*"))
		                             : FText::GetEmpty());
	return FText::Format(LOCTEXT("HBMontageGraphAssetEditorToolkitName", "{HBMontageGraphName}{DirtyState}"), Args);
}

void FMontageGraphEditor::RebuildMontageGraph()
{
	if (!GraphBeingEdited || !GraphBeingEdited)
	{
		return;
	}

	if (auto Graph{CastChecked<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph)})
	{
		Graph->RebuildGraph();
	}
}

void FMontageGraphEditor::SaveAsset_Execute()
{
	if (GraphBeingEdited)
	{
		RebuildMontageGraph();
	}

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FMontageGraphEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	// if (GraphBeingEdited)
	// {
	// 	GraphBeingEdited->EditorGraph->GetSchema()->ForceVisualizationCacheClear();
	// }
}

void FMontageGraphEditor::CreatePropertyWidget()
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
		this, &FMontageGraphEditor::OnFinishedChangingProperties);
}

void FMontageGraphEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Collector.AddReferencedObject(GraphBeingEdited->EditorGraph);
	// Collector.AddReferencedObject(GraphBeingEdited);
	Collector.AddReferencedObject(GraphBeingEdited);
}

void FMontageGraphEditor::CreateEditorGraph()
{
	if (!GraphBeingEdited)
	{
		return;
	}

	if (!GraphBeingEdited)
	{
		GraphBeingEdited = NewObject<UMontageGraph>(GraphBeingEdited,
		                                                              UMontageGraph::StaticClass(),
		                                                              FName("MontageGraph"));
	}

	if (!GraphBeingEdited->EditorGraph)
	{
		// State Machine Graph
		GraphBeingEdited->EditorGraph = FBlueprintEditorUtils::CreateNewGraph(
			GraphBeingEdited,
			FName("ActionEdGraph"),
			UMontageGraphEdGraph::StaticClass(),
			UMontageGraphSchema::StaticClass()
		);
		GraphBeingEdited->EditorGraph->bAllowRenaming = false;
		GraphBeingEdited->EditorGraph->bAllowDeletion = false;

		// Give the schema a chance to fill out any required nodes (like the entry node)
		const UEdGraphSchema* GraphSchema = GraphBeingEdited->EditorGraph->GetSchema();
		GraphSchema->CreateDefaultNodesForGraph(*GraphBeingEdited->EditorGraph);
	}
}


FGraphPanelSelectionSet FMontageGraphEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	if (GraphEditorView.IsValid())
	{
		CurrentSelection = GraphEditorView->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FMontageGraphEditor::SelectAllNodes() const
{
	if (GraphEditorView.IsValid())
	{
		GraphEditorView->SelectAllNodes();
	}
}

bool FMontageGraphEditor::CanSelectAllNodes()
{
	return true;
}

void FMontageGraphEditor::DeleteSelectedNodes() const
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

		if (UMontageGraphEdNode* MontageGraphEdNode = Cast<UMontageGraphEdNode>(EdNode))
		{
			MontageGraphEdNode->Modify();

			const UEdGraphSchema* Schema = MontageGraphEdNode->GetSchema();
			if (Schema)
			{
				Schema->BreakNodeLinks(*MontageGraphEdNode);
			}

			MontageGraphEdNode->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}
	}
}

bool FMontageGraphEditor::CanDeleteNodes() const
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

void FMontageGraphEditor::DeleteSelectedDuplicateNodes() const
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
void FMontageGraphEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicateNodes();
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FMontageGraphEditor::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FMontageGraphEditor::CopySelectedNodes() const
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

		if (const UMontageGraphEdNodeEdge* MontageGraphEdNodeEdge = Cast<UMontageGraphEdNodeEdge>(*SelectedIter))
		{
			const UMontageGraphEdNode* StartNode = MontageGraphEdNodeEdge->GetStartNodeAsGraphNode();
			const UMontageGraphEdNode* EndNode = MontageGraphEdNodeEdge->GetEndNode();

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

bool FMontageGraphEditor::CanCopyNodes() const
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

void FMontageGraphEditor::PasteNodes()
{
	if (GraphEditorView.IsValid())
	{
		PasteNodesHere(GraphEditorView->GetPasteLocation());
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FMontageGraphEditor::PasteNodesHere(const FVector2D& Location)
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
bool FMontageGraphEditor::CanPasteNodes()
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

void FMontageGraphEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool FMontageGraphEditor::CanDuplicateNodes()
{
	return CanCopyNodes();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FMontageGraphEditor::OnRenameNode()
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

bool FMontageGraphEditor::CanRenameNodes() const
{
	if (!GraphBeingEdited)
	{
		return false;
	}

	if (IsPIESimulating())
	{
		return false;
	}

	// UMontageGraphEdGraph* HBActioGraphEd = Cast<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph);
	// check(HBActioGraphEd);

	return GetSelectedNodes().Num() == 1;
}

bool FMontageGraphEditor::CanCreateComment() const
{
	return GraphEditorView.IsValid() ? (GraphEditorView->GetNumberOfSelectedNodes() != 0) : false;
}

void FMontageGraphEditor::OnCreateComment() const
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
void FMontageGraphEditor::OnGraphSelectionChanged(const TSet<UObject*>& NewSelection)
{
	MG_ERROR(Verbose, TEXT("OnGraphSelectionChanged - %d"), NewSelection.Num())

	TArray<UObject*> SelectedNodes;
	TArray<UMontageGraphEdNode*> GraphNodes;
	for (UObject* Selection : NewSelection)
	{
		SelectedNodes.Add(Selection);

		if (UMontageGraphEdNode* Node = Cast<UMontageGraphEdNode>(Selection))
		{
			GraphNodes.Add(Node);
		}
	}

	if (SelectedNodes.Num() > 0)
	{
		DetailsView->SetObjects(SelectedNodes);
	} else
	{
		DetailsView->SetObject(GraphBeingEdited);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FMontageGraphEditor::OnGraphNodeDoubleClicked(UEdGraphNode* Node)
{
	MG_ERROR(Verbose, TEXT("OnGraphNodeDoubleClicked - %s"), *GetNameSafe(Node))

	// TODO: Handle rename of node on double click
	// or opening of animation editor
}

void FMontageGraphEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo,
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
