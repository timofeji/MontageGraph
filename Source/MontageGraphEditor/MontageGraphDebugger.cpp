#include "MontageGraphDebugger.h"

#include "Editor/UnrealEdEngine.h"
#include "MontageGraphEditor.h"
#include "UnrealEdGlobals.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "Graph/EdNodes/MGEdNode_Entry.h"
#include "Graph/EdNodes/MGEdNode_Montage.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphComponent.h"
#include "MontageGraph/MontageGraphDelegates.h"

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
	for (auto DebugNode : SelectedNodes)
	{
	}
}

bool FMontageGraphDebugger::IsTickable() const
{
	return IsDebuggerReady();
}

void FMontageGraphDebugger::Setup(UMontageGraph* InMontageGraphAsset,
                                  TSharedRef<FMontageGraphEditor, ESPMode::ThreadSafe> InEditorOwner)
{
	EditorOwner       = InEditorOwner;
	MontageGraphAsset = InMontageGraphAsset;
	KnownInstances.Reset();

#if WITH_EDITORONLY_DATA
	if (IsPIESimulating())
	{
		// Subscribe to delegates for future events.
		OnBeginPIE(GEditor->bIsSimulatingInEditor);

		// Components that were already running before the editor opened have already broadcast
		// OnGraphInitialized and won't do so again.  Scan all game-world actors now and
		// register any MontageGraphComponent that is using this asset.
		ForEachGameWorld([this](UWorld* World)
		{
			for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
			{
				if (UMontageGraphComponent* MGC = (*ActorIt)->FindComponentByClass<UMontageGraphComponent>())
				{
					if (MGC->GetGraph() == MontageGraphAsset)
					{
						// OnGraphInitialized handles deduplication (KnownInstances.AddUnique not
						// necessary here, but OnGraphInitialized won't duplicate — it only adds).
						OnGraphInitialized(MGC);
					}
				}
			}
		});
	}
#endif
}

bool FMontageGraphDebugger::IsDebuggerReady() const
{
	return bIsPIEActive;
}

AActor* FMontageGraphDebugger::GetSelectedActor() const
{
	return CurrentlyDebuggedActor.IsValid() ? CurrentlyDebuggedActor.Get() : nullptr;
}

void FMontageGraphDebugger::OnGraphInitialized(const UMontageGraphComponent* InitializedGraphComponent)
{
	if (!InitializedGraphComponent) { return; }

	TWeakObjectPtr<UMontageGraphComponent> WeakPtr = const_cast<UMontageGraphComponent*>(InitializedGraphComponent);

	// Guard against duplicates (can happen when scanning existing PIE actors at editor-open time)
	const bool bAlreadyKnown = KnownInstances.ContainsByPredicate(
		[&](const TWeakObjectPtr<UMontageGraphComponent>& Existing)
		{
			return Existing.Get() == InitializedGraphComponent;
		});
	if (!bAlreadyKnown)
	{
		KnownInstances.Add(WeakPtr);
	}

	// Auto-select first component if nothing is selected yet
	if (!CurrentlyDebuggedActor.IsValid() && InitializedGraphComponent->GetOwner())
	{
		CurrentlyDebuggedActor = InitializedGraphComponent->GetOwner();

		if (EditorOwner.IsValid())
		{
			EditorOwner.Pin()->RegenerateMenusAndToolbars();
		}
	}
}

void FMontageGraphDebugger::OnBeginPIE(const bool bIsSimulating)
{
	bIsPIEActive = true;
	if (EditorOwner.IsValid())
	{
		const TSharedPtr<FMontageGraphEditor> EditorOwnerPtr = EditorOwner.Pin();
		EditorOwnerPtr->RegenerateMenusAndToolbars();
		// EditorOwnerPtr->DebuggerUpdateGraph(true);
	}

	// remove these delegates first as we can get multiple calls to OnBeginPIE()
	USelection::SelectObjectEvent.RemoveAll(this);
	FMontageGraphDelegates::OnGraphNodeSelected.RemoveAll(this);
	FMontageGraphDelegates::OnGraphNodeLinked.RemoveAll(this);
	FMontageGraphDelegates::OnGraphReset.RemoveAll(this);
	FMontageGraphDelegates::OnGraphInitialized.RemoveAll(this);

	USelection::SelectObjectEvent.AddRaw(this, &FMontageGraphDebugger::OnObjectSelected);
	FMontageGraphDelegates::OnGraphNodeLinked.AddRaw(this, &FMontageGraphDebugger::OnGraphNodeEvaluated);
	FMontageGraphDelegates::OnGraphNodeSelected.AddRaw(this, &FMontageGraphDebugger::OnGraphNodeSelected);
	FMontageGraphDelegates::OnGraphReset.AddRaw(this, &FMontageGraphDebugger::OnGraphReset);
	FMontageGraphDelegates::OnGraphInitialized.AddRaw(this, &FMontageGraphDebugger::OnGraphInitialized);
}

void FMontageGraphDebugger::OnEndPIE(const bool bIsSimulating)
{
	SelectedNode = nullptr;
	EvaluatedNodes.Empty();
	SelectedNodes.Empty();
	KnownInstances.Empty();

	bIsPIEActive = false;
	if (EditorOwner.IsValid())
	{
		const TSharedPtr<FMontageGraphEditor> EditorOwnerPtr = EditorOwner.Pin();
		EditorOwnerPtr->RegenerateMenusAndToolbars();
		// EditorOwnerPtr->DebuggerUpdateGraph(false);
	}

	USelection::SelectObjectEvent.RemoveAll(this);
	FMontageGraphDelegates::OnGraphNodeSelected.RemoveAll(this);
	FMontageGraphDelegates::OnGraphNodeLinked.RemoveAll(this);
	FMontageGraphDelegates::OnGraphReset.RemoveAll(this);
	FMontageGraphDelegates::OnGraphInitialized.RemoveAll(this);
}

void FMontageGraphDebugger::OnPausePIE(const bool bIsSimulating)
{
#if WITH_EDITORONLY_DATA
	// // We might have paused while executing a sub-tree, so make sure that the editor is showing the correct tree
#endif
}

void FMontageGraphDebugger::OnObjectSelected(UObject* Object)
{
	if (Object && Object->IsSelected())
	{
		AActor* Actor = Cast<AActor>(Object);
		if (!Actor)
		{
			return;
		}

		// Only switch debug target if the actor has a known MontageGraphComponent
		for (const TWeakObjectPtr<UMontageGraphComponent>& Instance : KnownInstances)
		{
			if (Instance.IsValid() && Instance->GetOwner() == Actor)
			{
				CurrentlyDebuggedActor = Actor;

				// Clear stale debug state from previous actor
				SelectedNode = nullptr;
				EvaluatedNodes.Empty();
				SelectedNodes.Empty();

				if (EditorOwner.IsValid())
				{
					EditorOwner.Pin()->RegenerateMenusAndToolbars();
				}
				return;
			}
		}
	}
}


void FMontageGraphDebugger::OnGraphNodeSelected(const UMontageGraphComponent* EvaluatedGraphComponent,
                                                const UMGNode& EvaluatedNode)
{
	TWeakObjectPtr<UMontageGraphComponent> WeakPtr = const_cast<UMontageGraphComponent*>(EvaluatedGraphComponent);

	if (!WeakPtr.IsValid())
	{
		return;
	}

	// Filter: only process events from the currently debugged actor
	if (CurrentlyDebuggedActor.IsValid() && WeakPtr->GetOwner() != CurrentlyDebuggedActor.Get())
	{
		return;
	}

	//Find evaluated EdNode
	UMGEdNode* EvaluatedEdNode = nullptr;
	if (auto Graph = WeakPtr.Get()->GetGraph())
	{
		for (auto EdNode : Graph->EditorGraph->Nodes)
		{
			if (auto MGEdNode = Cast<UMGEdNode>(EdNode))
			{
				if (MGEdNode->RuntimeNode == &EvaluatedNode)
				{
					EvaluatedEdNode = MGEdNode;
					// EvaluatedEdNode->DebugEvaluate();

					break;
				}
			}
		}
	}

	SelectedNode = const_cast<UMGNode*>(&EvaluatedNode);


	if (!EvaluatedEdNode)
	{
		return;
	}

	SelectedNodes.Reset();
	SelectedNodes.Add(EvaluatedEdNode);

	UMGEdNode* CurEdNode = EvaluatedEdNode;
	while (Cast<UMGEdNode_Entry>(CurEdNode) == nullptr)
	{
		SelectedNodes.Add(CurEdNode );

		CurEdNode = Cast<UMGEdNode>(CurEdNode->GetInputPin()->LinkedTo[0]->GetOwningNode());
	}
}

void FMontageGraphDebugger::OnGraphNodeEvaluated(const UMontageGraphComponent* EvaluatedGraphComponent,
                                                 const UMGNode& EvaluatedNode)
{
	TWeakObjectPtr<UMontageGraphComponent> WeakPtr = const_cast<UMontageGraphComponent*>(EvaluatedGraphComponent);

	if (!WeakPtr.IsValid())
	{
		return;
	}

	// Filter: only process events from the currently debugged actor
	if (CurrentlyDebuggedActor.IsValid() && WeakPtr->GetOwner() != CurrentlyDebuggedActor.Get())
	{
		return;
	}

	UMGEdNode* EvaluatedEdNode = nullptr;
	if (auto Graph = WeakPtr.Get()->GetGraph())
	{
		for (auto EdNode : Graph->EditorGraph->Nodes)
		{
			if (auto MGEdNode = Cast<UMGEdNode>(EdNode))
			{
				if (MGEdNode->RuntimeNode == &EvaluatedNode)
				{
					EvaluatedEdNode = MGEdNode;
					EvaluatedEdNode->DebugEvaluate();

					break;
				}
			}
		}
	}

	SelectedNode = const_cast<UMGNode*>(&EvaluatedNode);
	if (EditorOwner.IsValid())
	{
		if (auto DopeSheet = EditorOwner.Pin()->GetAnimDopeSheetWidget())
		{
			if (const auto MGEdNode_Montage = Cast<UMGEdNode_Montage>(EvaluatedEdNode))
			{
				DopeSheet->SetSelection(MGEdNode_Montage);
			}
		}
	}


	if (!EvaluatedEdNode)
	{
		return;
	}

	EvaluatedNodes.Reset();
	EvaluatedNodes.Add(EvaluatedEdNode);

	UMGEdNode* CurEdNode = EvaluatedEdNode;
	while (Cast<UMGEdNode_Entry>(CurEdNode) == nullptr)
	{
		EvaluatedNodes.Add(CurEdNode );
		CurEdNode = Cast<UMGEdNode>(CurEdNode->GetInputPin()->LinkedTo[0]->GetOwningNode());
	}
}


void FMontageGraphDebugger::OnGraphReset(const UMontageGraphComponent& OwnerActionComponent)
{
	EvaluatedNodes.Empty();

	for (int32 i = KnownInstances.Num() - 1; i >= 0; i--)
	{
		UMontageGraphComponent* Task = KnownInstances[i].Get();
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

UMontageGraphComponent* FMontageGraphDebugger::GetDebuggedTargetActor()
{
	AActor* SelectedActor = GetSelectedActor();
	if (!SelectedActor)
	{
		return nullptr;
	}

	UMontageGraphComponent* Result = nullptr;
	for (TWeakObjectPtr<UMontageGraphComponent> Component : KnownInstances)
	{
		if (!Component.IsValid())
		{
			continue;
		}

		AActor* Avatar = Component->GetOwner();
		if (Avatar == SelectedActor)
		{
			Result = Component.Get();
		}
	}

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
	AActor* Actor = CurrentlyDebuggedActor.Get();
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
	if (SelectedActor)
	{
		USelection* SelectedActors = GEditor ? GEditor->GetSelectedActors() : nullptr;
		if (SelectedActors)
		{
			SelectedActors->DeselectAll();
		}

		CurrentlyDebuggedActor = SelectedActor;

		// Clear stale debug state from previous actor
		SelectedNode = nullptr;
		EvaluatedNodes.Empty();
		SelectedNodes.Empty();

		if (EditorOwner.IsValid())
		{
			const TSharedPtr<FMontageGraphEditor> EditorOwnerPtr = EditorOwner.Pin();
			EditorOwnerPtr->RegenerateMenusAndToolbars();
		}

		if (SelectedActors)
		{
			SelectedActors->Select(SelectedActor);
		}
	}
}

void FMontageGraphDebugger::GetMatchingInstances(TArray<UMontageGraphComponent*>& MatchingInstances)
{
	for (int32 i = KnownInstances.Num() - 1; i >= 0; i--)
	{
		UMontageGraphComponent* ActiveComponent = KnownInstances[i].Get();
		if (ActiveComponent == nullptr)
		{
			KnownInstances.RemoveAt(i);
			continue;
		}

		MatchingInstances.Add(ActiveComponent);
	}
}
