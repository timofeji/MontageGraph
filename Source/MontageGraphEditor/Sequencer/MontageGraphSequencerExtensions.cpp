#include "MontageGraphSequencerExtensions.h"
#include "ISequencerModule.h"
#include "MontageGraphEditorStyles.h"
#include "MontageGraphSharedPosesTrackEditor.h"
#include "..\Slate\SControlRigSharedPoseDialog.h"
#include "Tools/ControlRigPose.h"

#define LOCTEXT_NAMESPACE "FMontageGraphEditorModule"

void FMontageGraphSequencerCommands::RegisterCommands()
{
	UI_COMMAND(AddSharedPose, "Add Shared Pose", "Adds a Shared Pose at current frame",
	           EUserInterfaceActionType::ToggleButton, FInputChord());
}


void FMontageGraphSequencerExtensions::ExtendSequencerToolbar(FToolBarBuilder& ToolbarBuilder) const
{
	ToolbarBuilder.BeginSection("MontageGraphExtensions");
	ToolbarBuilder.AddToolBarButton(
		FMontageGraphSequencerCommands::Get().AddSharedPose,
		NAME_None,
		LOCTEXT("PoseLinkLabel", ""),
		LOCTEXT("PoseLinkTooltip", "Create a Shared Pose from current frame"),
		FSlateIcon(FMontageGraphEditorStyles::Get().GetStyleSetName(), "MontageGraph.PoseLink")
	);
	ToolbarBuilder.EndSection();
}

void FMontageGraphSequencerExtensions::ToggleSharedPoseDialog()
{
	FText TitleText = LOCTEXT("SharedPoseWindowTitle", "Shared Pose Editor");
	// Create the window to choose our options
	TSharedRef<SWindow> SharedPoseWindow = SNew(SWindow)
    		.Title(TitleText)
    		.HasCloseButton(true)
    		.SizingRule(ESizingRule::UserSized)
    		.ClientSize(FVector2D(400.0f, 600.0f))
    		.AutoCenter(EAutoCenter::PreferredWorkArea)
    		.SupportsMinimize(false);

	TSharedRef<SControlRigSharedPoseDialog> DialogWidget = SNew(SControlRigSharedPoseDialog, SequencerRef.Pin());
	SharedPoseWindow->SetContent(DialogWidget);

	FSlateApplication::Get().AddWindow(SharedPoseWindow);
}

bool FMontageGraphSequencerExtensions::CanAddSharedPose() const
{
	return true;
}

void FMontageGraphSequencerExtensions::OnSequencerCreated(TSharedRef<ISequencer> Sequencer)
{
	SequencerRef = Sequencer;

	OnChannelChangedHandle = SequencerRef.Pin()->OnMovieSceneDataChanged().AddRaw(
		this, &FMontageGraphSequencerExtensions::OnMovieSceneDataChanged);
}

void FMontageGraphSequencerExtensions::Register()
{
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	OnSequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(
		FOnSequencerCreated::FDelegate::CreateSP(this, &FMontageGraphSequencerExtensions::OnSequencerCreated));

	TSharedPtr<FExtender> Extender = MakeShareable(new FExtender);

	FMontageGraphSequencerCommands::Register();

	TSharedPtr<FUICommandList> UICommandList = MakeShared<FUICommandList>();
	UICommandList->MapAction(FMontageGraphSequencerCommands::Get().AddSharedPose,
	                         FExecuteAction::CreateSP(this, &FMontageGraphSequencerExtensions::ToggleSharedPoseDialog),
	                         FCanExecuteAction::CreateSP(this, &FMontageGraphSequencerExtensions::CanAddSharedPose)
	);


	Extender->AddToolBarExtension(
		"BaseCommands",
		EExtensionHook::After,
		UICommandList,
		FToolBarExtensionDelegate::CreateRaw(this, &FMontageGraphSequencerExtensions::ExtendSequencerToolbar)
	);


	// SequencerModule.GetSequencerCustomizationManager()->RegisterInstancedSequencerCustomization(
	// 	UMovieSceneSequence::StaticClass(), FOnGetSequencerCustomizationInstance::CreateLambda([]()
	// 	{
	// 		return new FMontageGraphSequenceCustomization;
	// 	}));
	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(Extender);
	SharedPosesTrackHandle = SequencerModule.RegisterTrackEditor(
		FOnCreateTrackEditor::CreateStatic(&FSharedPosesTrackEditor::CreateTrackEditor));
}

void FMontageGraphSequencerExtensions::Unregister()
{
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnregisterOnSequencerCreated(OnSequencerCreatedHandle);
	OnSequencerCreatedHandle.Reset();

	SequencerModule.UnRegisterTrackEditor(SharedPosesTrackHandle);
}

void FMontageGraphSequencerExtensions::OnChannelChanged(const FMovieSceneChannelMetaData* MovieSceneChannelMetaData,
                                                        UMovieSceneSection* MovieSceneSection)
{
}

void FMontageGraphSequencerExtensions::OnMovieSceneDataChanged(EMovieSceneDataChangeType MovieSceneDataChange)
{
	// TWeakObjectPtr<UControlRigPoseAsset> PoseAsset;
	//
	// FControlRigEditMode* ControlRigEditMode = OwningControlRigWidget->GetEditMode();
	// if (ControlRigEditMode)
	// {
	// 	TMap<UControlRig*, TArray<FRigElementKey>> AllSelectedControls;
	// 	ControlRigEditMode->GetAllSelectedControls(AllSelectedControls);
	// 	if (AllSelectedControls.Num() == 1)
	// 	{
	// 		TArray<UControlRig*> ControlRigs;
	// 		AllSelectedControls.GenerateKeyArray(ControlRigs);
	// 		UControlRig* ControlRig = ControlRigs[0];
	// 		UObject* NewAsset = nullptr;
	// 	}
	// }
}

#undef LOCTEXT_NAMESPACE
