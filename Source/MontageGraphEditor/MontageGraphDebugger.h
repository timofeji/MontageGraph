// Copyright 2022 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UHBActionComponent;
class FMontageGraphEditor;
class UHBMontageGraphNodeBase;
class UMontageGraph;
class UMontageGraphNode;

class MONTAGEGRAPHEDITOR_API FMontageGraphDebugger : public FTickableGameObject
{
public:
	FMontageGraphDebugger();
	~FMontageGraphDebugger();

	//~ Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FComboGraphEditorTickHelper, STATGROUP_Tickables); }
	//~ End FTickableGameObject

	/** Refresh the debugging information we are displaying (only when paused, as Tick() updates when running) */
	void Setup(UMontageGraph* InHBMontageGraphAsset, TSharedRef<class FMontageGraphEditor, ESPMode::ThreadSafe> InEditorOwner);

	bool IsDebuggerReady() const;
	bool IsDebuggerRunning() const;

	AActor* GetSelectedActor() const;

	void OnGraphNodeSelected(const UMontageGraphNode& SelectedNode);
	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);
	void OnPausePIE(const bool bIsSimulating);

	void OnObjectSelected(UObject* Object);
	void OnGraphEvaluated(const UHBActionComponent& ActionComponent, const UMontageGraphNode& EvaluatedNode);
	// void OnHBMontageGraphStarted(const UHBMontageGraphAbilityTask_StartGraph& InOwnerTask, const UHBMontageGraph& InComboGraphAsset);
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

	TArray<const UMontageGraphNode*> SelectedNodesDebug;
	// TArray<const UHBMontageGraphNode*> SelectedNodes;
private:

	/** owning editor */
	TWeakPtr<FMontageGraphEditor> EditorOwner;

	/** asset for debugging */
	UMontageGraph* HBMontageGraphAsset;

	/** root node in asset's graph */
	TWeakObjectPtr<UHBMontageGraphNodeBase> RootNode;

	/** instance for debugging */
	TWeakObjectPtr<AActor> HBComponentOwner;

	/** all known ComboGraph task instances, cached for dropdown list */
	TArray<TWeakObjectPtr<UHBActionComponent>> KnownInstances;

	/** all known ComboGraph owner instances, cached for dropdown list */
	TArray<TWeakObjectPtr<AActor>> KnownActors;

	/** cached PIE state */
	bool bIsPIEActive = false;
};

