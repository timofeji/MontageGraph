#include "MontageGraphDebugger.h"

#include "Editor/UnrealEdEngine.h"
#include "MontageGraphEditor.h"
#include "MontageGraphEditorLog.h"
#include "UnrealEdGlobals.h"
#include "Engine/Selection.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphDelegates.h"
#include "MontageGraph/MontageGraphNode_Entry.h"

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

FMontageGraphDebugger::FMontageGraphDebugger()
{
	MontageGraphAsset = nullptr;
	bIsPIEActive = false;

	FEditorDelegates::BeginPIE.AddRaw(this, &FMontageGraphDebugger::OnBeginPIE);
	FEditorDelegates::EndPIE.AddRaw(this, &FMontageGraphDebugger::OnEndPIE);
	FEditorDelegates::PausePIE.AddRaw(this, &FMontageGraphDebugger::OnPausePIE);
}

FMontageGraphDebugger::~FMontageGraphDebugger()
{
	FEditorDelegates::BeginPIE.RemoveAll(this);
	FEditorDelegates::EndPIE.RemoveAll(this);
	FEditorDelegates::PausePIE.RemoveAll(this);
	USelection::SelectObjectEvent.RemoveAll(this);
}

void FMontageGraphDebugger::Tick(float DeltaTime)
{
	// MG_ERROR(Verbose, TEXT("FMontageGraphDebugger Tick"))
}

bool FMontageGraphDebugger::IsTickable() const
{
	return IsDebuggerReady();
}

void FMontageGraphDebugger::Setup(UMontageGraph* InMontageGraphAsset,
                                   TSharedRef<FMontageGraphEditor, ESPMode::ThreadSafe> InEditorOwner)
{
	EditorOwner = InEditorOwner;
	MontageGraphAsset = InMontageGraphAsset;
	KnownInstances.Reset();
	KnownActors.Reset();

#if WITH_EDITORONLY_DATA
	if (IsPIESimulating())
	{
		OnBeginPIE(GEditor->bIsSimulatingInEditor);
	}
#endif
}

bool FMontageGraphDebugger::IsDebuggerReady() const
{
	return bIsPIEActive;
}

bool FMontageGraphDebugger::IsDebuggerRunning() const
{
	return MontageGraphComponentOwner.IsValid();
}

AActor* FMontageGraphDebugger::GetSelectedActor() const
{
	return MontageGraphComponentOwner.IsValid() ? MontageGraphComponentOwner.Get() : nullptr;
}

void FMontageGraphDebugger::OnGraphNodeSelected(const UMontageGraphNode& SelectedNode)
{
	// SelectedNodes.Add(&SelectedNode);
}

void FMontageGraphDebugger::OnBeginPIE(const bool bIsSimulating)
{
	bIsPIEActive = true;
	if (EditorOwner.IsValid())
	{
		const TSharedPtr<FMontageGraphEditor> EditorOwnerPtr = EditorOwner.Pin();
		EditorOwnerPtr->RegenerateMenusAndToolbars();
		EditorOwnerPtr->DebuggerUpdateGraph(true);
	}

	// remove these delegates first as we can get multiple calls to OnBeginPIE()
	USelection::SelectObjectEvent.RemoveAll(this);
	FMontageGraphDelegates::OnGraphNodeEvaluated.RemoveAll(this);
	FMontageGraphDelegates::OnGraphReset.RemoveAll(this);

	USelection::SelectObjectEvent.AddRaw(this, &FMontageGraphDebugger::OnObjectSelected);
	FMontageGraphDelegates::OnGraphNodeEvaluated.AddRaw(this, &FMontageGraphDebugger::OnGraphEvaluated);
	FMontageGraphDelegates::OnGraphNodeSelected.AddRaw(this, &FMontageGraphDebugger::OnGraphNodeSelected);
	FMontageGraphDelegates::OnGraphReset.AddRaw(this, &FMontageGraphDebugger::OnGraphReset);
}

void FMontageGraphDebugger::OnEndPIE(const bool bIsSimulating)
{
	SelectedNodesDebug.Empty();
	KnownInstances.Empty();

	bIsPIEActive = false;
	if (EditorOwner.IsValid())
	{
		const TSharedPtr<FMontageGraphEditor> EditorOwnerPtr = EditorOwner.Pin();
		EditorOwnerPtr->RegenerateMenusAndToolbars();
		EditorOwnerPtr->DebuggerUpdateGraph(false);
	}

	USelection::SelectObjectEvent.RemoveAll(this);
	FMontageGraphDelegates::OnGraphNodeEvaluated.RemoveAll(this);
	FMontageGraphDelegates::OnGraphReset.RemoveAll(this);
}

void FMontageGraphDebugger::OnPausePIE(const bool bIsSimulating)
{
#if WITH_EDITORONLY_DATA
	// // We might have paused while executing a sub-tree, so make sure that the editor is showing the correct tree
#endif
}

void FMontageGraphDebugger::OnObjectSelected(UObject* Object)
{
	MG_ERROR(Verbose, TEXT("FMontageGraphDebugger TestDebugger OnObjectSelected %s"), *GetNameSafe(Object))

	if (Object && Object->IsSelected())
	{
		AActor* Actor = Cast<AActor>(Object);
		if (Actor)
		{
			MG_ERROR(
				Verbose, TEXT("FMontageGraphDebugger TestDebugger OnObjectSelected Update actor instance %s"),
				*GetNameSafe(Actor))
			MontageGraphComponentOwner = Actor;
		}
	}
}


void FMontageGraphDebugger::OnGraphEvaluated(const UHBActionComponent& EvaluateTask,
                                              const UMontageGraphNode& EvaluatedNode)
{
	// const bool bAssetMatches = HBMontageGraphAsset && HBMontageGraphAsset == &InHBMontageGraphAsset;
	// MG_ERROR(Verbose, TEXT("FMontageGraphDebugger TestDebugger OnHBMontageGraphStarted %s - %d (Ability: %s)"), *GetNameSafe(&InHBMontageGraphAsset), InHBMontageGraphAsset.GetUniqueID(), *GetNameSafe(&InOwnerTask))
	// MG_ERROR(Verbose, TEXT("FMontageGraphDebugger TestDebugger OnHBMontageGraphStarted bAssetMatches %s)"), bAssetMatches ? TEXT("true") : TEXT("false"))
	//
	// // start debugging if combo graph asset matches, and no other actor was selected
	// if (!MontageGraphComponentOwner.IsValid() && bAssetMatches)
	// {
	// 	AActor* Avatar = InOwnerTask.Character;
	//
	// 	MG_ERROR(Verbose, TEXT("FMontageGraphDebugger TestDebugger OnHBMontageGraphStarted SetObjeSetActorBeingDebuggedctBeingDebugged %s"), *GetNameSafe(Avatar))
	// 	MontageGraphComponentOwner = MakeWeakObjectPtr(Avatar);
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

	const UMontageGraphNode* Node = &EvaluatedNode;
	while (Cast<UMontageGraphNode_Entry>(Node) == nullptr)
	{
		SelectedNodesDebug.Add(Node);
		Node = Node->ParentNodes[0];
	}
	// EvaluateTask.OnCancelled.AddRaw(this, &FMontageGraphDebugger::OnGraphCancelled);	
}


void FMontageGraphDebugger::OnGraphReset(const UHBActionComponent& OwnerActionComponent)
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

UHBActionComponent* FMontageGraphDebugger::GetDebuggedTaskForSelectedActor()
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


bool FMontageGraphDebugger::IsPlaySessionPaused()
{
	return AreAllGameWorldPaused();
}

bool FMontageGraphDebugger::IsPlaySessionRunning()
{
	return !AreAllGameWorldPaused();
}

bool FMontageGraphDebugger::IsPIESimulating()
{
	return GEditor->bIsSimulatingInEditor || GEditor->PlayWorld;
}

bool FMontageGraphDebugger::IsPIENotSimulating()
{
	return !GEditor->bIsSimulatingInEditor && (GEditor->PlayWorld == nullptr);
}

FString FMontageGraphDebugger::GetDebuggedInstanceDesc() const
{
	AActor* Actor = MontageGraphComponentOwner.Get();
	if (Actor)
	{
		return DescribeInstance(*Actor);
	}

	return NSLOCTEXT("MontageGraphAssetEditor", "DebugActorNothingSelected", "No debug object selected").ToString();
}

FString FMontageGraphDebugger::GetActorLabel(const AActor* InActor, const bool bIncludeNetModeSuffix,
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

FString FMontageGraphDebugger::DescribeInstance(const AActor& ActorToDescribe) const
{
	return FString::Printf(TEXT("%s"), *GetActorLabel(&ActorToDescribe));
}

void FMontageGraphDebugger::OnInstanceSelectedInDropdown(AActor* SelectedActor)
{
	MG_ERROR(Verbose, TEXT("FMontageGraphDebugger OnInstanceSelectedInDropdown Actor: %s"),
	              *GetNameSafe(SelectedActor))

	if (SelectedActor)
	{
		USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr;
		if (SelectedActors)
		{
			SelectedActors->DeselectAll();
		}

		MontageGraphComponentOwner = SelectedActor;

		if (SelectedActors)
		{
			SelectedActors->Select(SelectedActor);
		}
	}
}

void FMontageGraphDebugger::GetMatchingInstances(TArray<UHBActionComponent*>& MatchingInstances,
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
