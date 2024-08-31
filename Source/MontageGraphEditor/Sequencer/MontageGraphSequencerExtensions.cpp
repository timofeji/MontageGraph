#include "MontageGraphSequencerExtensions.h"
#include "ISequencerModule.h"
#include "MontageGraphEditorStyles.h"
#include "MontageGraphSharedPosesTrackEditor.h"
#include "MovieScene.h"
#include "..\Slate\SControlRigSharedPoseDialog.h"

#define LOCTEXT_NAMESPACE "FMontageGraphEditorModule"

void FMontageGraphSequencerCommands::RegisterCommands()
{
	UI_COMMAND(AddSharedPose, "Add Shared Pose", "Adds a Shared Pose at current frame",
	           EUserInterfaceActionType::Button, FInputChord());
}


void FMontageGraphSequencerExtensions::ExtendSequencerToolbar(FToolBarBuilder& ToolbarBuilder) const
{
	ToolbarBuilder.AddToolBarButton(
		FMontageGraphSequencerCommands::Get().AddSharedPose,
		NAME_None,
		LOCTEXT("PoseLinkLabel", ""),
		LOCTEXT("PoseLinkTooltip", "Create a Shared Pose from current frame"),
		FSlateIcon(FMontageGraphEditorStyles::Get().GetStyleSetName(), "MontageGraph.PoseLink")
	);
}

void FMontageGraphSequencerExtensions::AddSharedPose() const
{
	FText TitleText = LOCTEXT("CreateSharedPose", "Create Shared Pose");
	// Create the window to choose our options
	TSharedRef<SWindow> Window = SNew(SWindow)
    		.Title(TitleText)
    		.HasCloseButton(true)
    		.SizingRule(ESizingRule::UserSized)
    		.ClientSize(FVector2D(400.0f, 200.0f))
    		.AutoCenter(EAutoCenter::PreferredWorkArea)
    		.SupportsMinimize(false);

	TSharedRef<SControlRigSharedPoseDialog> DialogWidget = SNew(SControlRigSharedPoseDialog);
	// DialogWidget->SetDelegate(InDelegate);
	Window->SetContent(DialogWidget);

	FSlateApplication::Get().AddWindow(Window);
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
	                         FExecuteAction::CreateSP(this, &FMontageGraphSequencerExtensions::AddSharedPose),
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
}

#undef LOCTEXT_NAMESPACE
