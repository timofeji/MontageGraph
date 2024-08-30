#include "MontageGraphEdNodeMontage.h"
#include "LevelSequence.h"
#include "AnimSequenceLevelSequenceLink.h"
#include "ILevelSequenceEditorToolkit.h"
#include "ISequencerModule.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/MontageGraphNode_Animation.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"

#define LOCTEXT_NAMESPACE "MontageGraphEdNodeMontage"

UMontageGraphEdNodeMontage::UMontageGraphEdNodeMontage()
{
	bCanRenameNode = true;
}


void UMontageGraphEdNodeMontage::OpenLinkedAnimation() const
{
	UMontageGraphNode_Animation* AnimNode = Cast<UMontageGraphNode_Animation>(RuntimeNode);
	if (AnimNode && AnimNode->AnimationMontage != nullptr)
	{
		TArray<UAnimationAsset*> AnimSequences;
		AnimNode->AnimationMontage->GetAllAnimationSequencesReferred(AnimSequences);

		if (!AnimSequences.Num())
		{
			return;
		}

		UAnimSequence* SequenceToOpen = Cast<UAnimSequence>(AnimSequences[0]);
		if (IInterface_AssetUserData* AnimAssetUserData = Cast<IInterface_AssetUserData>(SequenceToOpen))
		{
			UAnimSequenceLevelSequenceLink* AnimLevelLink = AnimAssetUserData->GetAssetUserData<
				UAnimSequenceLevelSequenceLink>();
			if (AnimLevelLink)
			{
				ULevelSequence* LevelSequence = AnimLevelLink->ResolveLevelSequence();
				if (LevelSequence)
				{
					
					//Open Sequencer
					GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(LevelSequence);
					
					IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->
					                                             FindEditorForAsset(
						                                             LevelSequence, true);

					const ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(
						AssetEditor);
					const TWeakPtr<ISequencer> WeakSequencer = LevelSequenceEditor
						                                           ? LevelSequenceEditor->GetSequencer()
						                                           : nullptr;
					const TSharedPtr<ISequencer> Sequencer = WeakSequencer.Pin();
					if (Sequencer.IsValid())
					{
						TArray<UMovieSceneTrack*> CurSelection;
						Sequencer->GetSelectedTracks(CurSelection);

						if (CurSelection.Num() > 0)
						{
							Sequencer->SelectTrack(CurSelection[0]);
						}
						else
						{
							//Try to select Skeletal Track if we don't have anything selected
							const UMovieScene* MovieScene = LevelSequence->GetMovieScene();
							if (UMovieSceneTrack* SkeletalTrack = MovieScene->FindTrack<
								UMovieSceneControlRigParameterTrack>(AnimLevelLink->SkelTrackGuid))
							{
								Sequencer->SelectTrack(SkeletalTrack);
							}
						}
					}
				}
			}
		}
	}
}

UAnimSequence* UMontageGraphEdNodeMontage::GetLinkedAnimation() const
{
	UMontageGraphNode_Animation* AnimNode = Cast<UMontageGraphNode_Animation>(RuntimeNode);
	if (AnimNode && AnimNode->AnimationMontage != nullptr)
	{
		TArray<UAnimationAsset*> AnimSequences;
		AnimNode->AnimationMontage->GetAllAnimationSequencesReferred(AnimSequences);

		if (!AnimSequences.Num())
		{
			return nullptr;
		}

		UAnimSequence* LinkedAnimSeq = Cast<UAnimSequence>(AnimSequences[0]);
		return LinkedAnimSeq;
	}

	return nullptr;
}

bool UMontageGraphEdNodeMontage::HasLinkedMontage() const
{
	UMontageGraphNode_Animation* AnimNode = Cast<UMontageGraphNode_Animation>(RuntimeNode);
	if (AnimNode && AnimNode->AnimationMontage != nullptr)
	{
		TArray<UAnimationAsset*> AnimSequences;
		AnimNode->AnimationMontage->GetAllAnimationSequencesReferred(AnimSequences);

		if (!AnimSequences.Num())
		{
			return false;
		}

		if (IInterface_AssetUserData* AnimAssetUserData = Cast<IInterface_AssetUserData>(AnimSequences[0]))
		{
			UAnimSequenceLevelSequenceLink* AnimLevelLink = AnimAssetUserData->GetAssetUserData<
				UAnimSequenceLevelSequenceLink>();
			if (AnimLevelLink)
			{
				ULevelSequence* LevelSequence = AnimLevelLink->ResolveLevelSequence();
				if (LevelSequence)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void UMontageGraphEdNodeMontage::OnRenameNode(const FString& NewName)
{
	AnimationName = NewName;
	Modify(true);
}

void UMontageGraphEdNodeMontage::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMontageGraphPinNames::PinName_In, TEXT("In"));
	CreatePin(EGPD_Output, UMontageGraphPinNames::PinName_Out, TEXT("Out"));
}

void UMontageGraphEdNodeMontage::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin)
	{
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

FText UMontageGraphEdNodeMontage::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return AnimationName.IsEmpty() ? LOCTEXT("Action Montage", "Montage") : FText::FromString(AnimationName);
}

FText UMontageGraphEdNodeMontage::GetTooltipText() const
{
	return LOCTEXT("Action Montage Tooltip",
	               "This is a Selector, which allows branching out beginning of Montage Graph based on an initial Transition Input");
}

void UMontageGraphEdNodeMontage::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}

FString UMontageGraphEdNodeMontage::GetDocumentationExcerptName() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("UAnimStateNode");
}

FString UMontageGraphEdNodeMontage::GetDocumentationLink() const
{
	// TODO: Figure out a way to do our own extensive documentation (when holding Ctrl+Alt when hovering)
	return TEXT("Shared/GraphNodes/AnimationStateMachine");
}

#undef LOCTEXT_NAMESPACE
