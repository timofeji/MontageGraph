// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "IMontageGraphEditor.h"
#include "Slate/SMontageGraphDopeSheet.h"

class UMontageGraph;
class UMGNode_Montage;
class IPersonaPreviewScene;
class IPersonaToolkit;
class IPersonaViewport;


struct FMontageGraphEditorModes
{
	// Mode constants
	static const FName Rules;
	static const FName Blends;
	static const FName Debug;

	static FText GetLocalizedMode(const FName InMode)
	{
		static TMap<FName, FText> LocModes;

		if (LocModes.Num() == 0)
		{
			LocModes.Add(Rules,
			             NSLOCTEXT("MGEditorModes", "MGEditorModes_Rules", "Rules"));
			
			LocModes.Add(Blends,
			             NSLOCTEXT("MGEditorModes", "MGEditorModes_EditBlends", "Blends"));
			
			LocModes.Add(Debug,
			             NSLOCTEXT("MGEditorModes", "MGEditorModes_Debug", "Debug"));
			
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


class FMontageGraphEditor : public IMontageGraphEditor, public FGCObject, public FNotifyHook,
                            public FEditorUndoClient, public FTickableEditorObject
{
public:
	~FMontageGraphEditor();
	void CreateInternalWidgets();

	void OnSectionSelected(UObject* Object);
	void OnDopeSheetUpdated();
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
		return TEXT("FMontageGraphAssetEditor");
	}

	
	//~ Begin FTickableEditorObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
	//~ End FTickableEditorObject Interface

	void AddPersonaToolbar();
	bool IsPIESimulating() const;
	void CreateDefaultCommands();
	void BindToolkitCommands();

	void InitMontageGraphEditor(EToolkitMode::Type              Mode,
	                            const TSharedPtr<IToolkitHost>& InitToolkitHost,
	                            UMontageGraph*                  GraphToEdit);

	void HandlePreviewSceneCreated(const TSharedRef<IPersonaPreviewScene>& InPersonaPreviewScene);
	void HandleOnPreviewSceneSettingsCustomized(IDetailLayoutBuilder& DetailLayoutBuilder);
	void HandlePreviewMeshChanged(USkeletalMesh* SkeletalMesh, USkeletalMesh* SkeletalMesh1);

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
	
	
	bool CanAccessGameplayMode() const;
	bool CanAccessBlendsMode() const;

	UMontageGraph* GetGraphBeingEdited() const { return GraphBeingEdited; }


	UAnimMontage*                    SelectedMontage;
	TSharedPtr<FDopeSheetController> TimelineController;


	static const FName ViewportTabID;
	static const FName GraphViewportTabID;
	static const FName AnimTimelineTabID;
	static const FName ActionNameTabID;
	static const FName DetailsTabID;
	static const FName PaletteTabID;

	// IToolkit interface
	//

	TSharedRef<SDockTab> SpawnTab_AssetBrowser(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnTab_GraphViewport(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnTab_AnimTimeline(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& SpawnTabArgs) const;
	
	
	FEdGraphPinType GetTargetPinType() const;
	void HandlePinTypeChanged(const FEdGraphPinType& EdGraphPin);

	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	FText GetBaseToolkitName() const override;
	FText GetToolkitName() const override;
	FName GetToolkitFName() const override { return FName(TEXT("MontageGraphEditor")); }

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
		return FString(TEXT("MontageGraphEditor"));
	}

	bool OnShouldFilterAsset(const FAssetData& AssetData);

	void RebuildStaleMontages();
	void LoadMontageGraph();

	virtual void SaveAsset_Execute() override;

	/** The command list for this asset editor with common functionality such as copy paste and so on */
	TSharedPtr<FUICommandList> DefaultCommands;
private:
	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;
	TSharedPtr<IPropertyHandle> TypePropertyHandle;
	FEdGraphPinType LastGraphPinType;
	
	TSharedPtr<class FMontageGraphEditorToolbar> ToolbarBuilder;
	int ActionIndex = 0;


	TSharedPtr<IPersonaToolkit> PersonaToolkit;

	AStaticMeshActor* WeakGroundActorPtr;



	void DebuggerUpdateGraph(bool bIsPIEActive);

	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);
	void CreatePropertyWidget();

	// The custom details view used
	TSharedPtr<IDetailsView> DetailsView;

	TSharedPtr<SGraphEditor> GraphEditorView;
	
	TSharedPtr<class SMontageGraphDopeSheet> AnimDopeSheet;
	
	TSharedPtr<class FAnimTimeSliderController> TimeSliderController;

	TSharedPtr<SBox> AssetBrowserBox;

	void TogglePlayback(bool bShouldPlay);
	

public:
	/** Access the toolbar builder for this editor */
	TSharedPtr<class FMontageGraphEditorToolbar> GetToolbarBuilder() { return ToolbarBuilder; }
	
	/** Get the Graph Editor Widget */
	TSharedPtr<SGraphEditor> GetGraphEditorWidget() const { return GraphEditorView; }
	
	/** Get the Details Widget */
	TSharedPtr<IDetailsView> GetPropertyDetailsWidget() const { return DetailsView; }
	
	TSharedRef<IPersonaToolkit> GetPersonaToolkit() const override
	{
		return PersonaToolkit.ToSharedRef();
	}

protected:
	UMontageGraph* GraphBeingEdited;
private:
	/** The skeleton we are editing */
	TObjectPtr<USkeletalMesh> SkeletalMesh;

};
