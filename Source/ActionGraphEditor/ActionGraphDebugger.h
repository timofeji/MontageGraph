// Copyright 2022 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UHBActionComponent;
class FActionGraphEditor;
class UHBActionGraphNodeBase;
class UActionGraph;
class UActionGraphNode;

class ACTIONGRAPHEDITOR_API FActionGraphDebugger : public FTickableGameObject
{
public:
	FActionGraphDebugger();
	~FActionGraphDebugger();

	//~ Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FComboGraphEditorTickHelper, STATGROUP_Tickables); }
	//~ End FTickableGameObject

	/** Refresh the debugging information we are displaying (only when paused, as Tick() updates when running) */
	void Setup(UActionGraph* InHBActionGraphAsset, TSharedRef<class FActionGraphEditor, ESPMode::ThreadSafe> InEditorOwner);

	bool IsDebuggerReady() const;
	bool IsDebuggerRunning() const;

	AActor* GetSelectedActor() const;

	void OnGraphNodeSelected(const UActionGraphNode& SelectedNode);
	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);
	void OnPausePIE(const bool bIsSimulating);

	void OnObjectSelected(UObject* Object);
	void OnGraphEvaluated(const UHBActionComponent& ActionComponent, const UActionGraphNode& EvaluatedNode);
	// void OnHBActionGraphStarted(const UHBActionGraphAbilityTask_StartGraph& InOwnerTask, const UHBActionGraph& InComboGraphAsset);
	void OnGraphReset(const UHBActionComponent& OwnerActionComponent);
	UHBActionComponent* GetDebuggedTaskForSelectedActor();

	static bool IsPlaySessionPaused();
	static bool IsPlaySessionRunning();
	static bool IsPIESimulating();
	static bool IsPIENotSimulating();

	FString GetDebuggedInstanceDesc() const;
	FString GetActorLabel(const AActor* InActor, const bool bIncludeNetModeSuffix = true, const bool bIncludeSpawnedContext = true) const;
	FString DescribeInstance(const AActor& ActorToDescribe) const;
	void OnInstanceSelectedInDropdown(AActor* SelectedActor);
	void GetMatchingInstances(TArray<UHBActionComponent*>& MatchingInstances, TArray<AActor*>& MatchingActors);

	TArray<const UActionGraphNode*> SelectedNodesDebug;
	// TArray<const UHBActionGraphNode*> SelectedNodes;
private:

	/** owning editor */
	TWeakPtr<FActionGraphEditor> EditorOwner;

	/** asset for debugging */
	UActionGraph* HBActionGraphAsset;

	/** root node in asset's graph */
	TWeakObjectPtr<UHBActionGraphNodeBase> RootNode;

	/** instance for debugging */
	TWeakObjectPtr<AActor> HBComponentOwner;

	/** all known ComboGraph task instances, cached for dropdown list */
	TArray<TWeakObjectPtr<UHBActionComponent>> KnownInstances;

	/** all known ComboGraph owner instances, cached for dropdown list */
	TArray<TWeakObjectPtr<AActor>> KnownActors;

	/** cached PIE state */
	bool bIsPIEActive = false;
};

