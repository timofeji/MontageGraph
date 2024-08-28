// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "IMontageGraphEditor.h"

class UMontageGraph;
class UMontageGraphNode_Action;
class IPersonaPreviewScene;
class IPersonaToolkit;
class IPersonaViewport;

class AHBWeapon;



struct FMontageGraphEditorModes
{
	// Mode constants
	static const FName HBCombatEditorMode;

	static FText GetLocalizedMode(const FName InMode)
	{
		static TMap<FName, FText> LocModes;

		if (LocModes.Num() == 0)
		{
			LocModes.Add(HBCombatEditorMode, NSLOCTEXT("HBCombatEditorModes", "HBCombatEditorMode", "Combat"));
		}

		check(InMode != NAME_None);
		const FText* OutDesc = LocModes.Find(InMode);
		check(OutDesc);
		return *OutDesc;
	}

private:
	FMontageGraphEditorModes()
	{
	}
};


class FMontageGraphEditor : public IMontageGraphEditor, public FGCObject, public FNotifyHook
{
public:
	FMontageGraphEditor();
	~FMontageGraphEditor();
	void CreateInternalWidgets();

	void OnSelectedNodesChanged(const TSet<UObject*>& Objects) const;
	void OnNodeTitleCommitted(const FText& Text, ETextCommit::Type Arg, UEdGraphNode* EdGraphNode) const;
	void OnGraphActionMenuClosed(bool bArg, bool bCond) const;

	FActionMenuContent OnCreateGraphActionMenu(UEdGraph* EdGraph, const UE::Math::TVector2<double>& Vector2,
	                                           const TArray<UEdGraphPin*>& EdGraphPins,
	                                           bool bArg, TDelegate<void()> Delegate);

	void OnGraphSelectionChanged(const TSet<UObject*>& Objects);

	void OnGraphNodeDoubleClicked(UEdGraphNode* EdGraphNode);
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

	void CreateEditorGraph();


	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	virtual FString GetReferencerName() const override
	{
		return TEXT("FHBCharacterAssetEditor");
	}

	void ExtendMenu();
	void FillToolbar(FToolBarBuilder& ToolBarBuilder);
	void ExtendToolbar();
	bool IsPIESimulating() const;
	void CreateDefaultCommands();
	void BindToolkitCommands();

	void HandlePreviewSceneCreated(const TSharedRef<IPersonaPreviewScene>& InPersonaPreviewScene);
	void InitCharacterEditor(EToolkitMode::Type Mode,
	                         const TSharedPtr<IToolkitHost>& InitToolkitHost,
	                         UMontageGraph* GraphToEdit);


	FGraphPanelSelectionSet GetSelectedNodes() const;


	TSharedRef<SWidget> OnGetDebuggerActorsMenu();

	
	
	void SelectAllNodes() const;
	bool CanSelectAllNodes();
	void DeleteSelectedNodes() const;
	bool CanDeleteNodes() const;
	void DeleteSelectedDuplicateNodes() const;
	void CutSelectedNodes();
	bool CanCutNodes();
	void CopySelectedNodes() const;
	bool CanCopyNodes() const;
	void PasteNodes();
	void PasteNodesHere(const FVector2D& Location);
	bool CanPasteNodes();
	void DuplicateNodes();
	bool CanDuplicateNodes();
	void OnRenameNode();
	bool CanRenameNodes() const;

	bool CanCreateComment() const;
	void OnCreateComment() const;

	TSharedPtr<class FBlueprintEditorToolbar> Toolbar;
	

	float SampleRate = 20.f;
	float TracerAnimTime = 0.1;
	FTimerHandle SampleTimerHandle;

	TSharedPtr<class FBlueprintEditorToolbar> GetToolbarBuilder() { return Toolbar; }


	static const FName ViewportTabID;
	static const FName MontageGraphTabID;
	static const FName ActionNameTabID;
	static const FName DetailsTabID;
	static const FName PaletteTabID;


	// // Refreshes the graph viewport.
	// void RefreshViewport() const
	// {
	// 	if (GraphEditorView.IsValid())
	// 	{
	// 		GraphEditorView->NotifyGraphChanged();
	// 	}
	// }

	// IToolkit interface
	//

	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& SpawnTabArgs) const;
	FEdGraphPinType GetTargetPinType() const;
	void HandlePinTypeChanged(const FEdGraphPinType& EdGraphPin);
	TSharedRef<SDockTab> SpawnTab_GraphViewport(const FSpawnTabArgs& SpawnTabArgs);

	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	FText GetBaseToolkitName() const override;
	FText GetToolkitName() const override;
	FName GetToolkitFName() const override { return FName(TEXT("HBCollectibleEditor")); }

	FText GetToolkitToolTipText() const override
	{
		return GetToolTipTextForObject(Cast<UObject>(GraphBeingEdited));
	}

	FLinearColor GetWorldCentricTabColorScale() const override
	{
		return FLinearColor::White;
	}

	FString GetWorldCentricTabPrefix() const override
	{
		return FString(TEXT("HBCollectibleEditor"));
	}


	void RebuildMontageGraph();

	virtual void SaveAsset_Execute() override;

	/** The command list for this asset editor with common functionality such as copy paste and so on */
	TSharedPtr<FUICommandList> DefaultCommands;

	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;
	TSharedPtr<IPropertyHandle> TypePropertyHandle;
	FEdGraphPinType LastGraphPinType;
	int ActionIndex = 0;


	TSharedPtr<IPersonaToolkit> PersonaToolkit;

	AStaticMeshActor* WeakGroundActorPtr;

	TSharedRef<IPersonaToolkit> GetPersonaToolkit() const override
	{
		return PersonaToolkit.ToSharedRef();
	}

	/** Called from Editor debugger and rebuilds exec graph for the Combo Graph Asset, and indicates if debugger is in active state (eg. PIE running) */
	void DebuggerUpdateGraph(bool bIsPIEActive);

	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);
	void CreatePropertyWidget();

	// The custom details view used
	TSharedPtr<IDetailsView> DetailsView;

	TSharedPtr<SGraphEditor> GraphEditorView;


	/** Get the Graph Editor Widget */
	TSharedPtr<SGraphEditor> GetGraphEditorWidget() const { return GraphEditorView; }
	TSharedPtr<IDetailsView> GetPropertyDetailsWidget() const { return DetailsView; }

protected:
	UMontageGraph* GraphBeingEdited;
};
