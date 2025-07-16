// Created by Timofej Jermolaev, All rights reserved. 

#pragma once

#include "CoreMinimal.h"

class UMGEdNode;
class FMontageGraphEditor;
class UMontageGraphComponent;
class UMontageGraphNodeBase;
class UMontageGraph;
class UMGNode;


UENUM()
enum EMontageGraphDebugMode:uint8
{
	MG_DEBUG_NONE,
	MG_DEBUG_TAGS,
};


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
	void Setup(UMontageGraph* InMontageGraphAsset, TSharedRef<class FMontageGraphEditor, ESPMode::ThreadSafe> InEditorOwner);

	bool IsDebuggerReady() const;

	AActor* GetSelectedActor() const;
	
	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);
	void OnPausePIE(const bool bIsSimulating);

	void OnObjectSelected(UObject* Object);
	
	void OnGraphInitialized(const UMontageGraphComponent* InitializedGraphComponent);
	void OnGraphNodeEvaluated(const UMontageGraphComponent* ActionComponent, const UMGNode& EvaluatedNode);
	void OnGraphNodeSelected(const UMontageGraphComponent* EvaluatedGraphComponent, const UMGNode& EvaluatedNode);
	// void OnMontageGraphStarted(const UMontageGraphAbilityTask_StartGraph& InOwnerTask, const UMontageGraph& InComboGraphAsset);
	void OnGraphReset(const UMontageGraphComponent& OwnerActionComponent);
	UMontageGraphComponent* GetDebuggedTargetActor();

	static bool IsPlaySessionPaused();
	static bool IsPlaySessionRunning();
	static bool IsPIESimulating();
	static bool IsPIENotSimulating();

	FString GetDebuggedInstanceDesc() const;
	FString GetActorLabel(const AActor* InActor, const bool bIncludeNetModeSuffix = true, const bool bIncludeSpawnedContext = true) const;
	FString DescribeInstance(const AActor& ActorToDescribe) const;
	void OnInstanceSelectedInDropdown(AActor* SelectedActor);
	void GetMatchingInstances(TArray<UMontageGraphComponent*>& MatchingInstances);

	UMGNode* SelectedNode;
	TArray<const UMGEdNode*> EvaluatedNodes;
	TArray<const UMGEdNode*> SelectedNodes;

private:

	/** owning editor */
	TWeakPtr<FMontageGraphEditor> EditorOwner;

	/** Graph currently loaded in editor*/
	UMontageGraph* MontageGraphAsset;

	/** root node in asset's graph */
	TWeakObjectPtr<UMontageGraphNodeBase> RootNode;

	/** instance for debugging */
	TWeakObjectPtr<AActor> CurrentlyDebuggedActor;

	/** all known MontageGraphComponent instances, cached for debugging*/
	TArray<TWeakObjectPtr<UMontageGraphComponent>> KnownInstances;

	/** cached PIE state */
	bool bIsPIEActive = false;
	
	EMontageGraphDebugMode DebugMode = MG_DEBUG_NONE;
public:
	EMontageGraphDebugMode GetDebugMode() { return DebugMode; }
};

