#pragma once
#include "CoreMinimal.h"
#include "ISequencerModule.h"
#include "Editor/Experimental/EditorInteractiveToolsFramework/Public/Behaviors/2DViewportBehaviorTargets.h"

struct FMontageGraphSequencerExtensions : public TSharedFromThis<FMontageGraphSequencerExtensions>
{
	void ToggleSharedPoseDialog();
	bool CanAddSharedPose() const;
	void Register();
	void Unregister();

	void OnChannelChanged(const FMovieSceneChannelMetaData* MovieSceneChannelMetaData, UMovieSceneSection* MovieSceneSection);
	void OnMovieSceneDataChanged(EMovieSceneDataChangeType MovieSceneDataChange);
	void OnSequencerCreated(TSharedRef<ISequencer> Sequencer);
	void ExtendSequencerToolbar(FToolBarBuilder& ToolbarBuilder) const;
	
	TWeakPtr<ISequencer> SequencerRef;
	FDelegateHandle OnSequencerCreatedHandle;
	FDelegateHandle OnChannelChangedHandle;
	FDelegateHandle  SharedPosesTrackHandle;
};

class FMontageGraphSequencerCommands : public TCommands<FMontageGraphSequencerCommands>
{
public:
	FMontageGraphSequencerCommands()
		: TCommands<FMontageGraphSequencerCommands>
		(
			TEXT("MontageGraphSequncerCommands"),
			NSLOCTEXT("Contexts", "MontageGraphSequncerCommands", "Anim Sequence Montage Graph extensions"),
			NAME_None,
			FAppStyle::GetAppStyleSetName()
		)
	{
	}


	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> AddSharedPose;
};
