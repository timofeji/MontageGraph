#include "ActionGraphDebugger.h"

#include "Editor/UnrealEdEngine.h"
#include "ActionGraphEditor.h"
#include "ActionGraphEditorLog.h"
#include "UnrealEdGlobals.h"
#include "Engine/Selection.h"
#include "ActionGraph/ActionGraph.h"
#include "ActionGraph/ActionGraphDelegates.h"
#include "ActionGraph/ActionGraphNode_Entry.h"

static void ForEachGameWorld(const TFunction<void(UWorld*)>& Func)
{
	for (const FWorldContext& PieContext : GUnrealEd->GetWorldContexts())
	{
		UWorld* PlayWorld = PieContext.World();
		if (PlayWorld && PlayWorld->IsGameWorld())
		{
			Func(PlayWorld);
		}
	}
}

static bool AreAllGameWorldPaused()
{
	bool bPaused = true;
	ForEachGameWorld([&](const UWorld* World)
	{
		bPaused = bPaused && World->bDebugPauseExecution;
	});
	return bPaused;
}

FActionGraphDebugger::FActionGraphDebugger()
{
	HBActionGraphAsset = nullptr;
	bIsPIEActive = false;

	FEditorDelegates::BeginPIE.AddRaw(this, &FActionGraphDebugger::OnBeginPIE);
	FEditorDelegates::EndPIE.AddRaw(this, &FActionGraphDebugger::OnEndPIE);
	FEditorDelegates::PausePIE.AddRaw(this, &FActionGraphDebugger::OnPausePIE);
}

FActionGraphDebugger::~FActionGraphDebugger()
{
	FEditorDelegates::BeginPIE.RemoveAll(this);
	FEditorDelegates::EndPIE.RemoveAll(this);
	FEditorDelegates::PausePIE.RemoveAll(this);
	USelection::SelectObjectEvent.RemoveAll(this);
}

void FActionGraphDebugger::Tick(float DeltaTime)
{
	// ACTIONGRAPH_LOG(Verbose, TEXT("FActionGraphDebugger Tick"))
}

bool FActionGraphDebugger::IsTickable() const
{
	return IsDebuggerReady();
}

void FActionGraphDebugger::Setup(UActionGraph* InHBActionGraphAsset,
                                   TSharedRef<FActionGraphEditor, ESPMode::ThreadSafe> InEditorOwner)
{
	EditorOwner = InEditorOwner;
	HBActionGraphAsset = InHBActionGraphAsset;
	KnownInstances.Reset();
	KnownActors.Reset();

#if WITH_EDITORONLY_DATA
	if (IsPIESimulating())
	{
		OnBeginPIE(GEditor->bIsSimulatingInEditor);
	}
#endif
}

bool FActionGraphDebugger::IsDebuggerReady() const
{
	return bIsPIEActive;
}

bool FActionGraphDebugger::IsDebuggerRunning() const
{
	return HBComponentOwner.IsValid();
}

AActor* FActionGraphDebugger::GetSelectedActor() const
{
	return HBComponentOwner.IsValid() ? HBComponentOwner.Get() : nullptr;
}

void FActionGraphDebugger::OnGraphNodeSelected(const UActionGraphNode& SelectedNode)
{
	// SelectedNodes.Add(&SelectedNode);
}

void FActionGraphDebugger::OnBeginPIE(const bool bIsSimulating)
{
	bIsPIEActive = true;
	if (EditorOwner.IsValid())
	{
		const TSharedPtr<FActionGraphEditor> EditorOwnerPtr = EditorOwner.Pin();
		EditorOwnerPtr->RegenerateMenusAndToolbars();
		EditorOwnerPtr->DebuggerUpdateGraph(true);
	}

	// remove these delegates first as we can get multiple calls to OnBeginPIE()
	USelection::SelectObjectEvent.RemoveAll(this);
	FActionGraphDelegates::OnGraphNodeEvaluated.RemoveAll(this);
	FActionGraphDelegates::OnGraphReset.RemoveAll(this);

	USelection::SelectObjectEvent.AddRaw(this, &FActionGraphDebugger::OnObjectSelected);
	FActionGraphDelegates::OnGraphNodeEvaluated.AddRaw(this, &FActionGraphDebugger::OnGraphEvaluated);
	FActionGraphDelegates::OnGraphNodeSelected.AddRaw(this, &FActionGraphDebugger::OnGraphNodeSelected);
	FActionGraphDelegates::OnGraphReset.AddRaw(this, &FActionGraphDebugger::OnGraphReset);
}

void FActionGraphDebugger::OnEndPIE(const bool bIsSimulating)
{
	SelectedNodesDebug.Empty();
	KnownInstances.Empty();

	bIsPIEActive = false;
	if (EditorOwner.IsValid())
	{
		const TSharedPtr<FActionGraphEditor> EditorOwnerPtr = EditorOwner.Pin();
		EditorOwnerPtr->RegenerateMenusAndToolbars();
		EditorOwnerPtr->DebuggerUpdateGraph(false);
	}

	USelection::SelectObjectEvent.RemoveAll(this);
	FActionGraphDelegates::OnGraphNodeEvaluated.RemoveAll(this);
	FActionGraphDelegates::OnGraphReset.RemoveAll(this);
}

void FActionGraphDebugger::OnPausePIE(const bool bIsSimulating)
{
#if WITH_EDITORONLY_DATA
	// // We might have paused while executing a sub-tree, so make sure that the editor is showing the correct tree
#endif
}

void FActionGraphDebugger::OnObjectSelected(UObject* Object)
{
	ACTIONGRAPH_LOG(Verbose, TEXT("FActionGraphDebugger TestDebugger OnObjectSelected %s"), *GetNameSafe(Object))

	if (Object && Object->IsSelected())
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor)
		{
			ACTIONGRAPH_LOG(
				Verbose, TEXT("FActionGraphDebugger TestDebugger OnObjectSelected Update actor instance %s"),
				*GetNameSafe(Actor))
			HBComponentOwner = Actor;
		}
	}
}


void FActionGraphDebugger::OnGraphEvaluated(const UHBActionComponent& EvaluateTask,
                                              const UActionGraphNode& EvaluatedNode)
{
	// const bool bAssetMatches = HBActionGraphAsset && HBActionGraphAsset == &InHBActionGraphAsset;
	// ACTIONGRAPH_LOG(Verbose, TEXT("FActionGraphDebugger TestDebugger OnHBActionGraphStarted %s - %d (Ability: %s)"), *GetNameSafe(&InHBActionGraphAsset), InHBActionGraphAsset.GetUniqueID(), *GetNameSafe(&InOwnerTask))
	// ACTIONGRAPH_LOG(Verbose, TEXT("FActionGraphDebugger TestDebugger OnHBActionGraphStarted bAssetMatches %s)"), bAssetMatches ? TEXT("true") : TEXT("false"))
	//
	// // start debugging if combo graph asset matches, and no other actor was selected
	// if (!HBComponentOwner.IsValid() && bAssetMatches)
	// {
	// 	AActor* Avatar = InOwnerTask.Character;
	//
	// 	ACTIONGRAPH_LOG(Verbose, TEXT("FActionGraphDebugger TestDebugger OnHBActionGraphStarted SetObjeSetActorBeingDebuggedctBeingDebugged %s"), *GetNameSafe(Avatar))
	// 	HBComponentOwner = MakeWeakObjectPtr(Avatar);
	// }
	//
	// // Update known instances if combo graph asset matches
	// if (bAssetMatches)
	// {
	// 	const TWeakObjectPtr<UHBActionComponent> OwnerTaskInstance = const_cast<UHBActionComponent*>(&InOwnerTask);
	// 	if (OwnerTaskInstance.IsValid())
	// 	{
	// 		KnownInstances.AddUnique(OwnerTaskInstance);
	// 	}
	//
	// 	const TWeakObjectPtr<AActor> OwnerActor = InOwnerTask.Character;
	// 	if (OwnerActor.IsValid())
	// 	{
	// 		KnownActors.AddUnique(OwnerActor);
	// 	}
	// }

	SelectedNodesDebug.Empty();

	const UActionGraphNode* Node = &EvaluatedNode;
	while (Cast<UActionGraphNode_Entry>(Node) == nullptr)
	{
		SelectedNodesDebug.Add(Node);
		Node = Node->ParentNodes[0];
	}
	// EvaluateTask.OnCancelled.AddRaw(this, &FActionGraphDebugger::OnGraphCancelled);	
}


void FActionGraphDebugger::OnGraphReset(const UHBActionComponent& OwnerActionComponent)
{
	SelectedNodesDebug.Empty();

	for (int32 i = KnownInstances.Num() - 1; i == 0; i--)
	{
		UHBActionComponent* Task = KnownInstances[i].Get();
		if (Task == nullptr)
		{
			KnownInstances.RemoveAt(i);
			continue;
		}

		if (Task == &OwnerActionComponent)
		{
			KnownInstances.RemoveAt(i);
		}
	}
}

UHBActionComponent* FActionGraphDebugger::GetDebuggedTaskForSelectedActor()
{
	AActor* SelectedActor = GetSelectedActor();
	if (!SelectedActor)
	{
		return nullptr;
	}

	UHBActionComponent* Result = nullptr;
	// for (TWeakObjectPtr<UHBActionComponent> Component : KnownInstances)
	// {
	// 	if (!Component.IsValid())
	// 	{
	// 		continue;
	// 	}
	//
	// 	AActor* Avatar = Component->GetOwner();
	// 	if (Avatar == SelectedActor)
	// 	{
	// 		Result = Component.Get();
	// 	}
	// }
	
	return Result;
}


bool FActionGraphDebugger::IsPlaySessionPaused()
{
	return AreAllGameWorldPaused();
}

bool FActionGraphDebugger::IsPlaySessionRunning()
{
	return !AreAllGameWorldPaused();
}

bool FActionGraphDebugger::IsPIESimulating()
{
	return GEditor->bIsSimulatingInEditor || GEditor->PlayWorld;
}

bool FActionGraphDebugger::IsPIENotSimulating()
{
	return !GEditor->bIsSimulatingInEditor && (GEditor->PlayWorld == nullptr);
}

FString FActionGraphDebugger::GetDebuggedInstanceDesc() const
{
	AActor* Actor = HBComponentOwner.Get();
	if (Actor)
	{
		return DescribeInstance(*Actor);
	}

	return NSLOCTEXT("HBActionGraphAssetEditor", "DebugActorNothingSelected", "No debug object selected").ToString();
}

FString FActionGraphDebugger::GetActorLabel(const AActor* InActor, const bool bIncludeNetModeSuffix,
                                              const bool bIncludeSpawnedContext) const
{
	if (!InActor)
	{
		return TEXT_NULL;
	}

	FString Context;
	FString Label = InActor->GetActorLabel();

	if (bIncludeNetModeSuffix)
	{
		// ReSharper disable once CppIncompleteSwitchStatement
		// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
		switch (InActor->GetNetMode())
		{
		case ENetMode::NM_Client:
			{
				Context = NSLOCTEXT("BlueprintEditor", "DebugWorldClient", "Client").ToString();

				FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(InActor->GetWorld());
				if (WorldContext != nullptr && WorldContext->PIEInstance > 1)
				{
					Context += TEXT(" ");
					Context += FText::AsNumber(WorldContext->PIEInstance - 1).ToString();
				}
			}
			break;

		case ENetMode::NM_ListenServer:
		case ENetMode::NM_DedicatedServer:
			Context = NSLOCTEXT("BlueprintEditor", "DebugWorldServer", "Server").ToString();
			break;
		}
	}

	if (bIncludeSpawnedContext)
	{
		if (!Context.IsEmpty())
		{
			Context += TEXT(", ");
		}

		Context += NSLOCTEXT("BlueprintEditor", "DebugObjectSpawned", "spawned").ToString();
	}

	if (!Context.IsEmpty())
	{
		Label = FString::Printf(TEXT("%s (%s)"), *Label, *Context);
	}

	return Label;
}

FString FActionGraphDebugger::DescribeInstance(const AActor& ActorToDescribe) const
{
	return FString::Printf(TEXT("%s"), *GetActorLabel(&ActorToDescribe));
}

void FActionGraphDebugger::OnInstanceSelectedInDropdown(AActor* SelectedActor)
{
	ACTIONGRAPH_LOG(Verbose, TEXT("FActionGraphDebugger OnInstanceSelectedInDropdown Actor: %s"),
	              *GetNameSafe(SelectedActor))

	if (SelectedActor)
	{
		USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr;
		if (SelectedActors)
		{
			SelectedActors->DeselectAll();
		}

		HBComponentOwner = SelectedActor;

		if (SelectedActors)
		{
			SelectedActors->Select(SelectedActor);
		}
	}
}

void FActionGraphDebugger::GetMatchingInstances(TArray<UHBActionComponent*>& MatchingInstances,
                                                  TArray<AActor*>& MatchingActors)
{
	for (int32 i = KnownInstances.Num() - 1; i >= 0; i--)
	{
		UHBActionComponent* ActionComponent = KnownInstances[i].Get();
		if (ActionComponent == nullptr)
		{
			KnownInstances.RemoveAt(i);
			continue;
		}

		MatchingInstances.Add(ActionComponent);
	}

	for (int32 i = KnownActors.Num() - 1; i >= 0; i--)
	{
		AActor* Actor = KnownActors[i].Get();
		if (Actor == nullptr)
		{
			KnownActors.RemoveAt(i);
			continue;
		}

		MatchingActors.Add(Actor);
	}
}
