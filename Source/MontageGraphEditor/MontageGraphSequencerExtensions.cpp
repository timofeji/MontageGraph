#include "MontageGraphSequencerExtensions.h"
#include "ISequencerModule.h"
#include "MontageGraphEditorStyles.h"

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
}

bool FMontageGraphSequencerExtensions::CanAddSharedPose() const
{
	return true;
}

void FMontageGraphSequencerExtensions::Register() const
{
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
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

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(Extender);
}

void FMontageGraphSequencerExtensions::Unregister() const
{
}

#undef LOCTEXT_NAMESPACE
