// Created by Timofej Jermolaev, All rights reserved . 

#include "MontageGraphEditor.h"

#include "AnimationEditorPreviewActor.h"
#include "AnimPose.h"
#include "AnimPreviewInstance.h"
#include "BlueprintEditor.h"
#include "ContentBrowserDataSource.h"
#include "ContentBrowserModule.h"
#include "EdGraphUtilities.h"

#include "EditorViewportCommands.h"
#include "GraphEditorActions.h"
#include "IContentBrowserSingleton.h"
#include "MontageGraphDebugger.h"
#include "MontageGraphEditorCommands.h"
#include "MontageGraphEditorStyle.h"
#include "MontageGraphEditorLog.h"
#include "PersonaModule.h"
#include "Framework/Commands/GenericCommands.h"
#include "GenericPlatform/GenericApplication.h"
#include "Graph/MontageGraphEdGraph.h"
#include "Graph/MontageGraphSchema.h"
#include "Graph/EdNodes/MGEdNode.h"
#include "Graph\EdNodes\MGEdNode_Edge.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "IPersonaToolkit.h"
#include "IPersonaPreviewScene.h"
#include "MaterialDomain.h"
#include "MontageGraphEditorModes.h"
#include "MontageGraphEditorToolbar.h"
#include "PersonaTabs.h"
#include "Animation/AnimMontage.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "DopeSheet/DopeSheetController.h"
#include "Editor/Experimental/EditorInteractiveToolsFramework/Public/Behaviors/2DViewportBehaviorTargets.h"


#include "Engine/StaticMeshActor.h"
#include "Graph/EdNodes/MGEdNode_Montage.h"


#include "Misc/ScopedSlowTask.h"

#include "HAL/Platform.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformCrt.h"

#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphComponent.h"
#include "MontageGraph/Nodes/MGEdge.h"
#include "Slate/SMontageGraphDopeSheet.h"
#include "Tracks/MontageTrack_CollisionCache.h"
#include "UObject/AssetRegistryTagsContext.h"


const FName FMontageGraphEditor::DetailsTabID(TEXT("MontageGraph_Details"));
const FName FMontageGraphEditor::ViewportTabID(TEXT("MontageGraph_Viewport"));
const FName FMontageGraphEditor::GraphViewportTabID(TEXT("MontageGraph_GraphViewport"));
const FName FMontageGraphEditor::AnimTimelineTabID(TEXT("MontageGraph_AnimTimeline"));

const FName FMontageGraphEditorModes::Rules("Selection");
const FName FMontageGraphEditorModes::Blends("Blends");
const FName FMontageGraphEditorModes::Debug("Debug");
#define LOCTEXT_NAMESPACE "FMontageGraphAssetEditor"

FMontageGraphEditor::~FMontageGraphEditor()
{
	//Make sure all delegate for preview mesh change are removed, by setting it to nullptr
	if (PersonaToolkit.IsValid())
	{
		constexpr bool bSetPreviewMeshInAsset = false;
		PersonaToolkit->SetPreviewMesh(nullptr, bSetPreviewMeshInAsset);
	}
}


void FMontageGraphEditor::InitMontageGraphEditor(EToolkitMode::Type Mode,
                                                 const TSharedPtr<IToolkitHost>&
                                                 InitToolkitHost,
                                                 UMontageGraph* GraphToEdit)
{
	check(GraphToEdit);
	GraphBeingEdited = GraphToEdit;


	CreateDefaultCommands();
	BindToolkitCommands();

	FPersonaToolkitArgs PersonaToolkitArgs;
	PersonaToolkitArgs.OnPreviewSceneCreated = FOnPreviewSceneCreated::FDelegate::CreateSP(
		this, &FMontageGraphEditor::HandlePreviewSceneCreated);
	PersonaToolkitArgs.OnPreviewSceneSettingsCustomized = FOnPreviewSceneSettingsCustomized::FDelegate::CreateSP(
		this, &FMontageGraphEditor::HandleOnPreviewSceneSettingsCustomized);
	PersonaToolkitArgs.bPreviewMeshCanUseDifferentSkeleton = true;

	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
	PersonaToolkit                = PersonaModule.CreatePersonaToolkit(GraphToEdit, PersonaToolkitArgs);


	// Set a default preview mesh, if any
	PersonaToolkit->SetPreviewMesh(GraphToEdit->GetPreviewMesh(), false);
	PersonaToolkit->GetPreviewScene()->SetDefaultAnimationMode(EPreviewSceneDefaultAnimationMode::Animation);
	PersonaToolkit->GetPreviewScene()->RegisterOnPreviewMeshChanged(
		FOnPreviewMeshChanged::CreateSP(this, &FMontageGraphEditor::HandlePreviewMeshChanged));

	FEditorViewportCommands::Register();
	FGenericCommands::Register();
	FGraphEditorCommands::Register();


	CreateEditorGraph();
	CreateInternalWidgets();
	CreatePropertyWidget();

	// Initialize the asset editor and spawn nothing (dummy layout)
	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar        = true;

	FAssetEditorToolkit::InitAssetEditor(Mode,
	                                     InitToolkitHost,
	                                     FName(TEXT("MontageGraphEditor")),
	                                     FTabManager::FLayout::NullLayout,
	                                     // StandaloneDefaultLayout,
	                                     bCreateDefaultStandaloneMenu,
	                                     bCreateDefaultToolbar,
	                                     GraphBeingEdited);


	UMontageGraphEdGraph* HBActioGraphEd = Cast<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph);
	check(HBActioGraphEd);

	HBActioGraphEd->Debugger = MakeShareable(new FMontageGraphDebugger);
	HBActioGraphEd->Debugger->Setup(GraphBeingEdited, SharedThis(this));


	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FMontageGraphEditorToolbar(SharedThis(this)));
	}

	AddApplicationMode(
		FMontageGraphEditorModes::Rules,
		MakeShareable(new FMontageGraphEditorMode_Selection(SharedThis(this))));

	AddApplicationMode(
		FMontageGraphEditorModes::Blends,
		MakeShareable(new FMontageGraphEditorMode_Blends(SharedThis(this))));

	AddApplicationMode(
		FMontageGraphEditorModes::Debug,
		MakeShareable(new FMontageGraphEditorMode_Debug(SharedThis(this))));

	SetCurrentMode(FMontageGraphEditorModes::Rules);

	// extend menus and toolbar
	AddPersonaToolbar();
	RegenerateMenusAndToolbars();
}

void FMontageGraphEditor::CreateInternalWidgets()
{
	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
		this, &FMontageGraphEditor::OnGraphSelectionChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FMontageGraphEditor::OnGraphNodeDoubleClicked);
	InEvents.OnTextCommitted     = FOnNodeTextCommitted::CreateSP(this, &FMontageGraphEditor::OnNodeTitleCommitted);

	UEdGraph* EditorGraph = GraphBeingEdited->EditorGraph;
	check(EditorGraph);


	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = FText::FromString(GraphBeingEdited->GetName());

	// Make full graph editor
	const bool bGraphIsEditable = EditorGraph->bEditable;
	GraphEditorView             = SNew(SGraphEditor)
		.AdditionalCommands(DefaultCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditorGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(true);

	TimelineController = MakeShareable(new FDopeSheetController);
	TimelineController->OnSectionSelected.AddRaw(this,  &FMontageGraphEditor::OnSectionSelected);
	TimelineController->OnTogglePlayback.AddRaw(this, &FMontageGraphEditor::TogglePlayback);
	TimelineController->OnTimeChanged.AddLambda([this](float NewTime)
	{
		if (UAnimInstance* AnimInstance = GetPersonaToolkit()->GetPreviewMeshComponent()->GetAnimInstance())
		{
			if (SelectedMontage)
			{
				AnimInstance->Montage_SetPosition(SelectedMontage, NewTime);
			}
		}
	});


	AnimDopeSheet = SNew(SMontageGraphDopeSheet, TimelineController)
		.OnUpdateNodes(this, &FMontageGraphEditor::RebuildStaleMontages);
	


	// setup filtering
	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassPaths.Add(UAnimMontage::StaticClass()->GetClassPathName());
	AssetPickerConfig.Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
	
	AssetPickerConfig.bCanShowClasses = true;
	AssetPickerConfig.bShowBottomToolbar = true;
	AssetPickerConfig.bAddFilterUI = true;
	AssetPickerConfig.SelectionMode = ESelectionMode::Single;
	AssetPickerConfig.DefaultFilterMenuExpansion = EAssetTypeCategories::Animation;
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &FMontageGraphEditor::OnShouldFilterAsset);
	// AssetPickerConfig.OnAssetDoubleClicked = FOnAssetSelected::CreateSP(this, &SRetargetExporterAssetBrowser::OnAssetDoubleClicked);
	// AssetPickerConfig.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);
	// AssetPickerConfig.bAllowNullSelection = false;
	// AssetPickerConfig.bFocusSearchBoxWhenOpened = false;
	AssetPickerConfig.bShowPathInColumnView = false;
	AssetPickerConfig.bShowTypeInColumnView = false;
	AssetPickerConfig.HiddenColumnNames.Add(ContentBrowserItemAttributes::ItemDiskSize.ToString());
	AssetPickerConfig.HiddenColumnNames.Add(ContentBrowserItemAttributes::VirtualizedData.ToString());
	AssetPickerConfig.HiddenColumnNames.Add(TEXT("Path"));
	AssetPickerConfig.HiddenColumnNames.Add(TEXT("Class"));
	AssetPickerConfig.HiddenColumnNames.Add(TEXT("RevisionControl"));

	// hide all asset registry columns by default (we only really want the name and path)
	UObject* AnimSequenceDefaultObject = UAnimSequence::StaticClass()->GetDefaultObject();
	FAssetRegistryTagsContextData TagsContext(AnimSequenceDefaultObject, EAssetRegistryTagsCaller::Uncategorized);
	AnimSequenceDefaultObject->GetAssetRegistryTags(TagsContext);
	for (const TPair<FName, UObject::FAssetRegistryTag>& TagPair : TagsContext.Tags)
	{
		AssetPickerConfig.HiddenColumnNames.Add(TagPair.Key.ToString());
	}

	// Also hide the type column by default (but allow users to enable it, so don't use bShowTypeInColumnView)
	AssetPickerConfig.HiddenColumnNames.Add(TEXT("Class"));
	
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));


	AssetBrowserBox = SNew(SBox)
	[
		ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
	];
}

void FMontageGraphEditor::OnSectionSelected(UObject* Object)
{
	DetailsView->SetObject(Object);

	if (auto CollisionSection = Cast<UMontageTrackSection_CollisionCache>(Object))
	{
		if (UWorld* PreviewWorld = GetPersonaToolkit()->GetPreviewScene()->GetWorld())
		{
		}
	}
}

void FMontageGraphEditor::OnDopeSheetUpdated()
{
}


TStatId FMontageGraphEditor::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FPhysicsAssetEditor, STATGROUP_Tickables);
}

void FMontageGraphEditor::Tick(float DeltaTime)
{
	GetPersonaToolkit()->GetPreviewScene()->InvalidateViews();

	if (SelectedMontage)
	{
		if (UAnimInstance* AnimInstance = GetPersonaToolkit()->GetPreviewMeshComponent()->GetAnimInstance())
		{
			const float CurrentTime = AnimInstance->Montage_GetPosition(SelectedMontage);
			TimelineController->SetPlayHeadTime(CurrentTime, false);
		}
	}


	const auto*  EditorEdGraph = Cast<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph);
	if (EditorEdGraph && EditorEdGraph->Debugger.IsValid())
	{
		EditorEdGraph->Debugger->Tick(DeltaTime);
	}
}

bool FMontageGraphEditor::CanAccessGameplayMode() const
{
	return true;
}

bool FMontageGraphEditor::CanAccessBlendsMode() const
{
	return true;
}

void FMontageGraphEditor::AddPersonaToolbar()
{
	// If the ToolbarExtender is valid, remove it before rebuilding it
	if (ToolbarExtender.IsValid())
	{
		RemoveToolbarExtender(ToolbarExtender);
		ToolbarExtender.Reset();
	}


	FName              ParentName;
	static const FName MenuName = GetToolMenuToolbarName(ParentName);

	UToolMenu*            ToolMenu = UToolMenus::Get()->ExtendMenu(MenuName);
	const FToolMenuInsert SectionInsertLocation("Asset", EToolMenuInsertType::After);

	ToolMenu->AddDynamicSection("Persona", FNewToolMenuDelegate::CreateLambda([](UToolMenu* InToolMenu)
	{
		FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
		FPersonaModule::FCommonToolbarExtensionArgs Args;
		Args.bReferencePose = true;
		PersonaModule.AddCommonToolbarExtensions(InToolMenu, Args);
	}), SectionInsertLocation);


	ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ParentToolbarBuilder)
			{
				// Second toolbar on right side
				FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
				TSharedRef<class IAssetFamily> AssetFamily = PersonaModule.CreatePersonaAssetFamily(GraphBeingEdited);
				AddToolbarWidget(PersonaModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
			}
		));
	AddToolbarExtender(ToolbarExtender);
}

TSharedRef<SWidget> FMontageGraphEditor::OnGetDebuggerActorsMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	const auto*  EditorEdGraph = Cast<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph);
	if (!EditorEdGraph)
	{
		return MenuBuilder.MakeWidget();
	}

	if (EditorEdGraph && EditorEdGraph->Debugger.IsValid())
	{
		TSharedPtr<FMontageGraphDebugger> Debugger = EditorEdGraph->Debugger;

		TArray<UMontageGraphComponent*> MatchingInstances;
		Debugger->GetMatchingInstances(MatchingInstances);


		for (auto MatchingInstance : MatchingInstances)
		{
			AActor*                MatchingActor = MatchingInstance->GetOwner();
			TWeakObjectPtr<AActor> InstancePtr   = MatchingActor;

			FUIAction ItemAction(FExecuteAction::CreateLambda([InstancePtr, EditorEdGraph]()
			{
				EditorEdGraph->Debugger->OnInstanceSelectedInDropdown(InstancePtr.Get());
			}));


			const FText InstanceName = FText::FromString(Debugger->GetActorLabel(MatchingActor));
			MenuBuilder.AddMenuEntry(InstanceName, TAttribute<FText>(),
			                         FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage"),
			                         ItemAction);
		}


		// Failsafe when no components match
		if (MatchingInstances.Num() == 0)
		{
			const FText            ActorDesc = LOCTEXT("NoMatchForDebug", "Can't find matching actors");
			TWeakObjectPtr<AActor> InstancePtr;

			FUIAction ItemAction(FExecuteAction::CreateLambda([InstancePtr, EditorEdGraph]()
			{
				EditorEdGraph->Debugger->OnInstanceSelectedInDropdown(InstancePtr.Get());
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
	// GetToolkitCommands()->MapAction(FPersonaCommonCommands::Get().TogglePlay,
	// 	FExecuteAction::CreateRaw(&GetPersonaToolkit()->GetPreviewScene().Get(), &IPersonaPreviewScene::TogglePlayback));

	FMontageGraphEditorCommands::Register();
	// Auto Arrange commands
	// ToolkitCommands->MapAction(
	// 	FMontageGraphBlueprintEditorCommands::Get().AutoArrange,
	// 	FExecuteAction::CreateSP(this, &FMontageGraphEditor::HandleAutoArrange),
	// 	FCanExecuteAction::CreateSP(this, &FMontageGraphEditor::CanAutoArrange)
	// );
	//
	// ToolkitCommands->MapAction(
	// 	FMontageGraphBlueprintEditorCommands::Get().AutoArrangeVertical,
	// 	FExecuteAction::CreateSP(this, &FMontageGraphEditor::SetAndHandleAutoArrange, EHBActioGraphAutoArrangeStrategy::Vertical),
	// 	FCanExecuteAction::CreateSP(this, &FMontageGraphEditor::CanAutoArrange)
	// );

	// ToolkitCommands->MapAction(
	// 	FHBCharacterAssetEditorCommands::Get().RegenerateActionTracers,
	// 	FExecuteAction::CreateSP(this, &FMontageGraphEditor::RegenerateActionTracers),
	// 	FCanExecuteAction::CreateSP(this, &FMontageGraphEditor::CanRegenerateTracers)
	// );
}


void FMontageGraphEditor::HandlePreviewSceneCreated(
	const TSharedRef<IPersonaPreviewScene>& InPersonaPreviewScene)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	// 
	// load a ground mesh
	static const TCHAR* GroundAssetPath = TEXT("/MontageGraph/SM_COMMON_Plane_01.SM_COMMON_Plane_01");
	UStaticMesh*        FloorMesh       = Cast<UStaticMesh>(
		StaticLoadObject(UStaticMesh::StaticClass(), NULL, GroundAssetPath, NULL, LOAD_None, NULL));


	static const TCHAR* GroundMatAssetPath = TEXT(
		"/MontageGraph/M_MontageGraph_GroundPreview.M_MontageGraph_GroundPreview");
	UMaterial* GroundMaterial = Cast<UMaterial>(
		StaticLoadObject(UMaterial::StaticClass(), NULL, GroundMatAssetPath, NULL, LOAD_None, NULL));
	check(FloorMesh);
	check(GroundMaterial);


	UWorld* World = InPersonaPreviewScene->GetWorld();
	if (World)
	{
		World->bAllowAudioPlayback = false;

		FTransform GroundTransform = FTransform::Identity;
		GroundTransform.SetScale3D(FVector(3.f));

		// create ground mesh actor
		AStaticMeshActor* GroundActor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), GroundTransform);
		// GroundActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(10, 10, 1));
		GroundActor->SetFlags(RF_Transient);
		GroundActor->GetStaticMeshComponent()->SetStaticMesh(FloorMesh);
		GroundActor->GetStaticMeshComponent()->SetMaterial(0, GroundMaterial);
		GroundActor->SetMobility(EComponentMobility::Static);
		GroundActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GroundActor->GetStaticMeshComponent()->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		GroundActor->GetStaticMeshComponent()->bSelectable = false;
		// this will be an invisible collision box that users can use to test traces
		// GroundActor->GetStaticMeshComponent()->SetVisibility(false);

		WeakGroundActorPtr = GroundActor;
	}


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

	UMontageGraphComponent* MontageGraphComponent = NewObject<UMontageGraphComponent>(Actor);
	InPersonaPreviewScene->AddComponent(MontageGraphComponent, FTransform::Identity, true);
	InPersonaPreviewScene->SetAllowMeshHitProxies(false);
	InPersonaPreviewScene->SetAdditionalMeshesSelectable(false);
}

void FMontageGraphEditor::HandleOnPreviewSceneSettingsCustomized(IDetailLayoutBuilder& DetailLayoutBuilder)
{
}

void FMontageGraphEditor::HandlePreviewMeshChanged(USkeletalMesh* Old, USkeletalMesh* New)
{
	GraphBeingEdited->SetPreviewMesh(New);
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
	// if (bIsPIEActive)
	// {
	// 	RebuildMontageGraph();
	// }
}


TSharedRef<SDockTab> FMontageGraphEditor::SpawnTab_AnimTimeline(const FSpawnTabArgs& SpawnTabArgs)
{
	check(SpawnTabArgs.GetTabId() == FMontageGraphEditor::AnimTimelineTabID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("MontageGraphAnimTimelineTab_Title", "Montage Timeline"));

	if (AnimDopeSheet.IsValid())
	{
		SpawnedTab->SetContent(AnimDopeSheet.ToSharedRef());
	}

	return SpawnedTab;
}


TSharedRef<SDockTab> FMontageGraphEditor::SpawnTab_AssetBrowser(const FSpawnTabArgs& SpawnTabArgs)
{
	check(SpawnTabArgs.GetTabId() == FPersonaTabs::AssetBrowserID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("MontageGraphAnimAssetBrowserTab_Title", "Animation Asset Browser"));

	if (AssetBrowserBox.IsValid())
	{
		SpawnedTab->SetContent(AssetBrowserBox.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SDockTab> FMontageGraphEditor::SpawnTab_GraphViewport(const FSpawnTabArgs& SpawnTabArgs)
{
	check(SpawnTabArgs.GetTabId() == FMontageGraphEditor::GraphViewportTabID);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(LOCTEXT("MontageGraphViewportTab_Title", "GraphViewport"));

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
	
	
	InTabManager->RegisterTabSpawner(FPersonaTabs::AssetBrowserID,
	                                 FOnSpawnTab::CreateSP(this, &FMontageGraphEditor::SpawnTab_AssetBrowser))
	            .SetDisplayName(LOCTEXT("MontageGraphAssetBrowserTab", "AssetBrowser"))
	            .SetGroup(WorkspaceMenuCategoryRef)
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimSequence"));

	InTabManager->RegisterTabSpawner(FMontageGraphEditor::AnimTimelineTabID,
	                                 FOnSpawnTab::CreateSP(this, &FMontageGraphEditor::SpawnTab_AnimTimeline))
	            .SetDisplayName(LOCTEXT("MontageGraphAnimTimelineTab", "AnimTimeline"))
	            .SetGroup(WorkspaceMenuCategoryRef)
	            .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));


	InTabManager->RegisterTabSpawner(FMontageGraphEditor::DetailsTabID,
	                                 FOnSpawnTab::CreateSP(this, &FMontageGraphEditor::SpawnTab_Details))
	            .SetDisplayName(LOCTEXT("MontageGraphDetailsTab", "PropertyDetails"))
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
	Args.Add(TEXT("MontageGraphName"), FText::FromString(EditingObject->GetName()));
	Args.Add(TEXT("DirtyState"), EditingObject->GetOutermost()->IsDirty()
		                             ? FText::FromString(TEXT("*"))
		                             : FText::GetEmpty());
	return FText::Format(LOCTEXT("MontageGraphAssetEditorToolkitName", "{MontageGraphName}{DirtyState}"), Args);
}

bool FMontageGraphEditor::OnShouldFilterAsset(const FAssetData& InAssetData)
{
	if (PersonaToolkit.IsValid())
	{
		if (InAssetData.IsInstanceOf(UAnimationAsset::StaticClass()))
		{
			const USkeleton* DesiredSkeleton = GetPersonaToolkit()->GetPreviewMesh()->GetSkeleton();
			if (DesiredSkeleton)
			{
				return !DesiredSkeleton->IsCompatibleForEditor(InAssetData);
			}
		}
	}

	return false;
}

void FMontageGraphEditor::RebuildStaleMontages()
{
	if (!GraphBeingEdited || !GraphBeingEdited)
	{
		return;
	}

	if (auto Graph{CastChecked<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph)})
	{
		Graph->RebuildRuntimeGraph();

		// RegenerateActionTracers();
	}
}

void FMontageGraphEditor::LoadMontageGraph()
{
	if (!GraphBeingEdited || !GraphBeingEdited)
	{
		return;
	}

	if (auto Graph{CastChecked<UMontageGraphEdGraph>(GraphBeingEdited->EditorGraph)})
	{
		Graph->RebuildRuntimeGraph();

		// RegenerateActionTracers();
	}
}

void FMontageGraphEditor::SaveAsset_Execute()
{
	if (GraphBeingEdited)
	{
		RebuildStaleMontages();
	}

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FMontageGraphEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (GraphEditorView)
	{
		GraphEditorView->NotifyGraphChanged();
	}
}

void FMontageGraphEditor::CreatePropertyWidget()
{
	FDetailsViewArgs Args;
	Args.bUpdatesFromSelection  = false;
	Args.bLockable              = false;
	Args.bAllowSearch           = true;
	Args.NameAreaSettings       = FDetailsViewArgs::ActorsUseNameArea;
	Args.bHideSelectionTip      = true;
	Args.NotifyHook             = this;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView                           = PropertyModule.CreateDetailView(Args);
	DetailsView->SetObject(GraphBeingEdited);
	DetailsView->OnFinishedChangingProperties().AddSP(
		this, &FMontageGraphEditor::OnFinishedChangingProperties);
}

void FMontageGraphEditor::TogglePlayback(bool bShouldPlay)
{
	if (UAnimInstance* AnimInstance = GetPersonaToolkit()->GetPreviewMeshComponent()->GetAnimInstance())
	{
		if (SelectedMontage)
		{
			bShouldPlay ? AnimInstance->Montage_Resume(SelectedMontage) : AnimInstance->Montage_Pause(SelectedMontage);
		}
	}

	// if (auto DebugMeshComp = GetPersonaToolkit()->GetPreviewMeshComponent())
	// {
	// 	DebugMeshComp->PreviewInstance->SetPlaying(bShouldPlay);
	// }
	// if (UAnimInstance* AnimInstance = GetPersonaToolkit()->GetPreviewMeshComponent()->GetAnimInstance())
	//   		{
	//   			if (SelectedMontage)
	//   			{
	//   				AnimInstance->Montage(SelectedMontage, NewTime);
	//   			}
	//   		}
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
		//Create New Montage Graph
		GraphBeingEdited->EditorGraph = FBlueprintEditorUtils::CreateNewGraph(
			GraphBeingEdited,
			FName("MontageEdGraph"),
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

		if (UMGEdNode* MGEdNode = Cast<UMGEdNode>(EdNode))
		{
			MGEdNode->Modify();

			const UEdGraphSchema* Schema = MGEdNode->GetSchema();
			if (Schema)
			{
				Schema->BreakNodeLinks(*MGEdNode);
			}

			MGEdNode->DestroyNode();
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

		if (const UMGEdNode_Edge* MGEdNodeEdge = Cast<UMGEdNode_Edge>(*SelectedIter))
		{
			const UMGEdNode* StartNode = MGEdNodeEdge->GetStartNode();
			const UMGEdNode* EndNode   = MGEdNodeEdge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
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
		FPlatformApplicationMisc::ClipboardPaste(TextToImport);

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
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

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
	TArray<UObject*>   SelectedNodes;
	TArray<UMGEdNode*> GraphNodes;
	for (UObject* Selection : NewSelection)
	{
		SelectedNodes.Add(Selection);

		if (UMGEdNode* Node = Cast<UMGEdNode>(Selection))
		{
			GraphNodes.Add(Node);
		}

		if (UMGEdNode_Montage* MontageNode = Cast<UMGEdNode_Montage>(Selection))
		{
			if (GraphBeingEdited->Montages.IsValidIndex(MontageNode->RuntimeNode->ID))
			{
				SelectedMontage = GraphBeingEdited->Montages[MontageNode->RuntimeNode->ID];
				GetPersonaToolkit()->GetPreviewScene()->SetPreviewAnimationAsset(SelectedMontage);
				if (auto World = GetPersonaToolkit()->GetPreviewScene()->GetWorld())
				{
					World->bAllowAudioPlayback = false;
				}
			}
			break;
		}
	}


	if (SelectedNodes.Num() > 0)
	{
		DetailsView->SetObjects(SelectedNodes);

		if (auto MontageEdNode = Cast<UMGEdNode_Montage>(SelectedNodes[0]))
		{
			if (AnimDopeSheet.IsValid())
			{
				AnimDopeSheet->SetSelection(MontageEdNode);
				AnimDopeSheet->SetVisibility(EVisibility::Visible);
			}
		}
	}
	else
	{
		DetailsView->SetObject(GraphBeingEdited);
	}

}

// ReSharper disable once CppMemberFunctionMayBeConst
void FMontageGraphEditor::OnGraphNodeDoubleClicked(UEdGraphNode* Node)
{
	// TODO: Handle rename of node on double click
	// or opening of animation editor
}

void FMontageGraphEditor::OnNodeTitleCommitted(const FText&  NewText, ETextCommit::Type CommitInfo,
                                               UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		static const FText       TransactionTitle = FText::FromString(FString(TEXT("Rename Node")));
		const FScopedTransaction Transaction(TransactionTitle);
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}


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

#undef  LOCTEXT_NAMESPACE
