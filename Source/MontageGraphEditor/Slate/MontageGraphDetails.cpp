#include "MontageGraphDetails.h"

#include <Graph/Nodes/MontageGraphEdNodeMontage.h>

#include "AnimationEditorUtils.h"
#include "AnimSequenceLevelSequenceLink.h"
#include "AssetToolsModule.h"
#include "ControlRigObjectBinding.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "FileHelpers.h"
#include "LevelSequenceEditor/Public/ILevelSequenceEditorToolkit.h"
#include "Editor/Sequencer/Public/ISequencer.h"
#include "LevelEditorViewport.h"
#include "LevelSequence.h"
#include "LevelSequenceAnimSequenceLink.h"
#include "MontageGraphEditorLog.h"
#include "MovieScene.h"
#include "MovieSceneToolHelpers.h"
#include "SequencerChannelTraits.h"
#include "Animation/SkeletalMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ControlRigEditor/Public/EditMode/ControlRigEditMode.h"
#include "Editor/Sequencer/Private/Sequencer.h"
#include "Exporters/AnimSeqExportOption.h"
#include "Factories/AnimMontageFactory.h"
#include "Factories/AnimSequenceFactory.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Graph/MontageGraphEdGraph.h"
#include "Interfaces/IMainFrameModule.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphNode_Animation.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FMontageGraphDetails"

//Largely copied from ControlRigEditorModule.cpp
void FMontageGraphDetails::CreateLinkedControlRigAnimationForNode(UMontageGraphEdNodeMontage* AnimNode) const
{
	UWorld* World = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;

	if (!World)
	{
		MG_ERROR(Verbose, TEXT("Error: Attempting to create a linked control rig montage but no world is detected"));
		return;
	}

	const FString LevelSeqeuencePrefixStr = GraphBeingEdited->GraphOutputPrefix.IsEmpty()
		                                        ? TEXT("Driving")
		                                        : FString::Printf(
			                                        TEXT("Driving_%s"), *GraphBeingEdited->GraphOutputPrefix);

	const FString LevelSequenceName = FString::Printf(TEXT("%s_%s"),
	                                                  *LevelSeqeuencePrefixStr,
	                                                  *AnimNode->AnimationName);

	const FString LevelSequencePackagePath = GraphBeingEdited->GetOutermost()->GetName() / "Driving";

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString LevelSequenceUniquePackageName;
	FString LevelSequenceUniqueAssetName;
	AssetToolsModule.Get().CreateUniqueAssetName(LevelSequencePackagePath / LevelSequenceName, TEXT(""),
	                                             LevelSequenceUniquePackageName,
	                                             LevelSequenceUniqueAssetName);

	UPackage* LevelSequencePackage = CreatePackage(*LevelSequenceUniquePackageName);
	ULevelSequence* LevelSequence = NewObject<ULevelSequence>(LevelSequencePackage, *LevelSequenceUniqueAssetName,
	                                                          RF_Public | RF_Standalone);

	FAssetRegistryModule::AssetCreated(LevelSequence);

	LevelSequence->Initialize(); //creates movie scene
	LevelSequence->MarkPackageDirty();

	UMovieScene* MovieScene = LevelSequence->GetMovieScene();
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(LevelSequence);


	IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(
		LevelSequence, false);
	ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(AssetEditor);

	const TWeakPtr<ISequencer> WeakSequencer = LevelSequenceEditor ? LevelSequenceEditor->GetSequencer() : nullptr;
	const TSharedPtr<ISequencer> Sequencer = WeakSequencer.Pin();
	if (Sequencer.IsValid())
	{
		USkeletalMesh* SkelMesh = nullptr;
		if (IInterface_PreviewMeshProvider* PreviewMeshInterface = Cast<IInterface_PreviewMeshProvider>(
			GraphBeingEdited->ControlRigClass->ClassGeneratedBy))
		{
			SkelMesh = PreviewMeshInterface->GetPreviewMesh();
		}

		ASkeletalMeshActor* MeshActor = World->SpawnActor<ASkeletalMeshActor>(
			ASkeletalMeshActor::StaticClass(), FTransform::Identity);
		MeshActor->SetActorLabel(LevelSequenceName);
		if (SkelMesh)
		{
			MeshActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkelMesh);
		}
		MeshActor->RegisterAllComponents();
		TArray<TWeakObjectPtr<AActor>> ActorsToAdd;
		ActorsToAdd.Add(MeshActor);
		TArray<FGuid> ActorTracks = Sequencer->AddActors(ActorsToAdd, false);
		FGuid ActorTrackGuid = ActorTracks[0];

		// By default, convert this to a spawnable and delete the existing actor. If for some reason, 
		// the spawnable couldn't be generated, use the existing actor as a possessable (this could 
		// eventually be an option)
		TArray<FGuid> SpawnableGuids = Sequencer->ConvertToSpawnable(ActorTrackGuid);
		if (SpawnableGuids.Num())
		{
			ActorTrackGuid = SpawnableGuids[0];

			UObject* SpawnedMesh = Sequencer->FindSpawnedObjectOrTemplate(ActorTrackGuid);

			if (SpawnedMesh)
			{
				GCurrentLevelEditingViewportClient->GetWorld()->EditorDestroyActor(MeshActor, true);
				MeshActor = Cast<ASkeletalMeshActor>(SpawnedMesh);
				if (SkelMesh)
				{
					MeshActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkelMesh);
				}
				MeshActor->RegisterAllComponents();
			}
		}

		//Delete binding from default animating rig
		//if we have skel mesh component binding we can just delete that
		FGuid CompGuid = Sequencer->FindObjectId(*(MeshActor->GetSkeletalMeshComponent()),
		                                         Sequencer->GetFocusedTemplateID());
		if (CompGuid.IsValid())
		{
			if (!MovieScene->RemovePossessable(CompGuid))
			{
				MovieScene->RemoveSpawnable(CompGuid);
			}
		}
		else //otherwise if not delete the track
		{
			if (UMovieSceneTrack* ExistingTrack = MovieScene->FindTrack<UMovieSceneControlRigParameterTrack>(
				ActorTrackGuid))
			{
				MovieScene->RemoveTrack(*ExistingTrack);
			}
		}

		UMovieSceneControlRigParameterTrack* Track = MovieScene->AddTrack<UMovieSceneControlRigParameterTrack>(
			ActorTrackGuid);
		if (Track)
		{
			USkeletalMeshComponent* SkelMeshComp = MeshActor->GetSkeletalMeshComponent();
			USkeletalMesh* SkeletalMesh = SkelMeshComp->GetSkeletalMeshAsset();

			FString ObjectName = (GraphBeingEdited->ControlRigClass->GetName());
			ObjectName.RemoveFromEnd(TEXT("_C"));

			UControlRig* ControlRig = NewObject<UControlRig>(Track, GraphBeingEdited->ControlRigClass,
			                                                 FName(*ObjectName),
			                                                 RF_Transactional);
			ControlRig->SetObjectBinding(MakeShared<FControlRigObjectBinding>());
			ControlRig->GetObjectBinding()->BindToObject(MeshActor);
			ControlRig->GetDataSourceRegistry()->RegisterDataSource(UControlRig::OwnerComponent,
			                                                        ControlRig->GetObjectBinding()->GetBoundObject());
			ControlRig->Initialize();
			ControlRig->Evaluate_AnyThread();


			Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);


			Track->Modify();
			UMovieSceneSection* NewSection = Track->CreateControlRigSection(0, ControlRig, true);
			Track->SetTrackName(FName(*ObjectName));
			Track->SetDisplayName(FText::FromString(ObjectName));
			UMovieSceneControlRigParameterSection* ParamSection = Cast<UMovieSceneControlRigParameterSection>(
				NewSection);


			Sequencer->SelectTrack(Track);
			GCurrentLevelEditingViewportClient->FocusViewportOnBox(MeshActor->GetComponentsBoundingBox());

			Sequencer->EmptySelection();
			Sequencer->SelectSection(ParamSection);
			Sequencer->ThrobSectionSelection();
			Sequencer->ObjectImplicitlyAdded(ControlRig);
			Sequencer->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
			// FControlRigEditMode* ControlRigEditMode = static_cast<FControlRigEditMode*>(GLevelEditorModeTools().
			// 	GetActiveMode(FControlRigEditMode::ModeName));
			// if (!ControlRigEditMode)
			// {
			// 	GLevelEditorModeTools().ActivateMode(FControlRigEditMode::ModeName);
			// 	ControlRigEditMode = static_cast<FControlRigEditMode*>(GLevelEditorModeTools().GetActiveMode(
			// 		FControlRigEditMode::ModeName));
			// }
			// if (ControlRigEditMode)
			// {
			// 	ControlRigEditMode->AddControlRigObject(ControlRig, Sequencer);
			// }

			//create soft links to each other
			// if (IInterface_AssetUserData* AssetUserDataInterface = Cast<IInterface_AssetUserData>(LevelSequence))
			// {
			// 	ULevelSequenceAnimSequenceLink* LevelAnimLink = NewObject<ULevelSequenceAnimSequenceLink>(
			// 		LevelSequence, NAME_None, RF_Public | RF_Transactional);
			// 	FLevelSequenceAnimSequenceLinkItem LevelAnimLinkItem;
			// 	LevelAnimLinkItem.SkelTrackGuid = ActorTrackGuid;
			// 	// LevelAnimLinkItem.PathToAnimSequence = FSoftObjectPath(AnimSequence);
			// 	LevelAnimLinkItem.bExportMorphTargets = true;
			// 	LevelAnimLinkItem.bExportAttributeCurves = true;
			// 	LevelAnimLinkItem.Interpolation = EAnimInterpolationType::Linear;
			// 	LevelAnimLinkItem.CurveInterpolation = ERichCurveInterpMode::RCIM_Linear;
			// 	LevelAnimLinkItem.bExportMaterialCurves = true;
			// 	LevelAnimLinkItem.bExportTransforms = true;
			// 	LevelAnimLinkItem.bRecordInWorldSpace = false;
			// 	LevelAnimLinkItem.bEvaluateAllSkeletalMeshComponents = true;
			// 	LevelAnimLink->AnimSequenceLinks.Add(LevelAnimLinkItem);
			// 	AssetUserDataInterface->AddAssetUserData(LevelAnimLink);
			// }
			// if (IInterface_AssetUserData* AnimAssetUserData = Cast<IInterface_AssetUserData>(AnimSequence))
			// {
			// 	UAnimSequenceLevelSequenceLink* AnimLevelLink = AnimAssetUserData->GetAssetUserData<
			// 		UAnimSequenceLevelSequenceLink>();
			// 	if (!AnimLevelLink)
			// 	{
			// 		AnimLevelLink = NewObject<UAnimSequenceLevelSequenceLink>(
			// 			AnimSequence, NAME_None, RF_Public | RF_Transactional);
			// 		AnimAssetUserData->AddAssetUserData(AnimLevelLink);
			// 	}
			// 	AnimLevelLink->SetLevelSequence(LevelSequence);
			// 	AnimLevelLink->SkelTrackGuid = ActorTrackGuid;
			// }

			CreateLinkedAnimationAssets(AnimNode, AssetToolsModule, SkeletalMesh);

			//Instantly Save
			TArray<UPackage*> PackagesToSave;
			PackagesToSave.Add(LevelSequencePackage);
			FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false);
		}
	}
}

void FMontageGraphDetails::CreateLinkedAnimationAssets(UMontageGraphEdNodeMontage* AnimNode,
                                                       FAssetToolsModule& AssetToolsModule,
                                                       USkeletalMesh* SkeletalMesh) const
{
	if (SkeletalMesh)
	{
		const FString AnimSeqPrefixStr = GraphBeingEdited->GraphOutputPrefix.IsEmpty()
			                                 ? TEXT("Seq")
			                                 : FString::Printf(
				                                 TEXT("Seq_%s"), *GraphBeingEdited->GraphOutputPrefix);

		const FString AnimSequenceName = FString::Printf(TEXT("%s_%s"),
		                                                 *AnimSeqPrefixStr,
		                                                 *AnimNode->AnimationName);

		const FString AnimSeqPackagePath = GraphBeingEdited->GetOutermost()->GetName() / "Sequences";

		FString AnimSeqUniquePackageName;
		FString AnimSeqUniqueAssetName;
		AssetToolsModule.Get().CreateUniqueAssetName(AnimSeqPackagePath / AnimSequenceName, TEXT(""),
		                                             AnimSeqUniquePackageName,
		                                             AnimSeqUniqueAssetName);


		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");


		UAnimSequenceFactory* SeqFactory = NewObject<UAnimSequenceFactory>();
		SeqFactory->TargetSkeleton = SkeletalMesh->GetSkeleton();
		SeqFactory->PreviewSkeletalMesh = SkeletalMesh;

		UAnimSequence* NewAnimSequence = CastChecked<UAnimSequence>(
			AssetToolsModule.Get().CreateAsset(AnimSeqUniqueAssetName,
			                                   FPackageName::GetLongPackagePath(AnimSeqUniquePackageName),
			                                   UAnimSequence::StaticClass(), SeqFactory));

		if (NewAnimSequence == nullptr)
		{
			return;
		}

		const FString AnimMontagePrefixStr = GraphBeingEdited->GraphOutputPrefix.IsEmpty()
			                                     ? TEXT("Montage")
			                                     : FString::Printf(
				                                     TEXT("Montage_%s"), *GraphBeingEdited->GraphOutputPrefix);

		const FString AnimMontageName = FString::Printf(TEXT("%s_%s"),
		                                                *AnimMontagePrefixStr,
		                                                *AnimNode->AnimationName);

		const FString AnimMontagePackagePath = GraphBeingEdited->GetOutermost()->GetName() / "Montages";

		FString AnimMontageUniquePackageName;
		FString AnimMontageUniqueAssetName;
		AssetToolsModule.Get().CreateUniqueAssetName(AnimMontagePackagePath / AnimMontageName, TEXT(""),
		                                             AnimMontageUniquePackageName,
		                                             AnimMontageUniqueAssetName);


		UAnimMontageFactory* MontageFactory = NewObject<UAnimMontageFactory>();
		MontageFactory->TargetSkeleton = SkeletalMesh->GetSkeleton();
		MontageFactory->SourceAnimation = NewAnimSequence;
		MontageFactory->PreviewSkeletalMesh = SkeletalMesh;

		UAnimMontage* NewAnimMontage = CastChecked<UAnimMontage>(
			AssetToolsModule.Get().CreateAsset(AnimMontageUniqueAssetName,
			                                   FPackageName::GetLongPackagePath(AnimMontageUniquePackageName),
			                                   UAnimMontage::StaticClass(), MontageFactory));

		UMontageGraphNode_Animation* MontageNode = Cast<UMontageGraphNode_Animation>(AnimNode->RuntimeNode);
		if (NewAnimMontage == nullptr || MontageNode == nullptr)
		{
			return;
		}

		MontageNode->AnimationMontage = NewAnimMontage;
	}
}

bool FMontageGraphDetails::CreateAnimationSequence(const TArray<UObject*> NewAssets,
                                                   USkeletalMeshComponent* SkelMeshComp, FGuid Binding,
                                                   bool bCreateSoftLink,
                                                   TSharedPtr<ISequencer> SequencerPtr) const
{
	// bool bResult = false;
	// if (NewAssets.Num() > 0)
	// {
	// 	UAnimSequence* AnimSequence = Cast<UAnimSequence>(NewAssets[0]);
	// 	if (AnimSequence == nullptr)
	// 	{
	// 		return false;
	// 	}
	// 	
	// 	UMovieScene* MovieScene = SequencerPtr->GetFocusedMovieSceneSequence()->GetMovieScene();
	// 	FMovieSceneSequenceIDRef Template = SequencerPtr->GetFocusedTemplateID();
	// 	FMovieSceneSequenceTransform RootToLocalTransform = SequencerPtr->
	// 		GetFocusedMovieSceneSequenceTransform();
	// 	
	// 	bResult = MovieSceneToolHelpers::ExportToAnimSequence(AnimSequence, AnimSeqExportOption, MovieScene,
	// 	                                                      SequencerPtr.Get(), SkelMeshComp, Template,
	// 	                                                      RootToLocalTransform);
	//
	// 	if (bCreateSoftLink)
	// 	{
	// 		FScopedTransaction Transaction(LOCTEXT("SaveLinkedAnimation_Transaction", "Save Link Animation"));
	// 		ULevelSequence* LevelSequence = Cast<ULevelSequence>(SequencerPtr->GetFocusedMovieSceneSequence());
	// 		if (LevelSequence && LevelSequence->GetClass()->ImplementsInterface(UInterface_AssetUserData::StaticClass())
	// 			&& AnimSequence->GetClass()->ImplementsInterface(UInterface_AssetUserData::StaticClass()))
	// 		{
	// 			LevelSequence->Modify();
	// 			if (IInterface_AssetUserData* AnimAssetUserData = Cast<IInterface_AssetUserData>(AnimSequence))
	// 			{
	// 				UAnimSequenceLevelSequenceLink* AnimLevelLink = AnimAssetUserData->GetAssetUserData<
	// 					UAnimSequenceLevelSequenceLink>();
	// 				if (!AnimLevelLink)
	// 				{
	// 					AnimLevelLink = NewObject<UAnimSequenceLevelSequenceLink>(
	// 						AnimSequence, NAME_None, RF_Public | RF_Transactional);
	// 					AnimAssetUserData->AddAssetUserData(AnimLevelLink);
	// 				}
	//
	// 				AnimLevelLink->SetLevelSequence(LevelSequence);
	// 				AnimLevelLink->SkelTrackGuid = Binding;
	// 			}
	// 			if (IInterface_AssetUserData* AssetUserDataInterface = Cast<IInterface_AssetUserData>(LevelSequence))
	// 			{
	// 				bool bAddItem = true;
	// 				ULevelSequenceAnimSequenceLink* LevelAnimLink = AssetUserDataInterface->GetAssetUserData<
	// 					ULevelSequenceAnimSequenceLink>();
	// 				if (LevelAnimLink)
	// 				{
	// 					for (FLevelSequenceAnimSequenceLinkItem& LevelAnimLinkItem : LevelAnimLink->AnimSequenceLinks)
	// 					{
	// 						if (LevelAnimLinkItem.SkelTrackGuid == Binding)
	// 						{
	// 							bAddItem = false;
	// 							UAnimSequence* OtherAnimSequence = LevelAnimLinkItem.ResolveAnimSequence();
	//
	// 							if (OtherAnimSequence != AnimSequence)
	// 							{
	// 								if (IInterface_AssetUserData* OtherAnimAssetUserData = Cast<
	// 									IInterface_AssetUserData>(OtherAnimSequence))
	// 								{
	// 									UAnimSequenceLevelSequenceLink* OtherAnimLevelLink = OtherAnimAssetUserData->
	// 										GetAssetUserData<UAnimSequenceLevelSequenceLink>();
	// 									if (OtherAnimLevelLink)
	// 									{
	// 										OtherAnimAssetUserData->RemoveUserDataOfClass(
	// 											UAnimSequenceLevelSequenceLink::StaticClass());
	// 									}
	// 								}
	// 							}
	// 							LevelAnimLinkItem.PathToAnimSequence = FSoftObjectPath(AnimSequence);
	// 							// LevelAnimLinkItem.bExportMorphTargets = AnimSeqExportOption->bExportMorphTargets;
	// 							// LevelAnimLinkItem.bExportAttributeCurves = AnimSeqExportOption->bExportAttributeCurves;
	// 							// LevelAnimLinkItem.bExportMaterialCurves = AnimSeqExportOption->bExportMaterialCurves;
	// 							// LevelAnimLinkItem.bExportTransforms = AnimSeqExportOption->bExportTransforms;
	// 							// LevelAnimLinkItem.bRecordInWorldSpace = AnimSeqExportOption->bRecordInWorldSpace;
	// 							// LevelAnimLinkItem.bEvaluateAllSkeletalMeshComponents = AnimSeqExportOption->
	// 							// 	bEvaluateAllSkeletalMeshComponents;
	// 							// LevelAnimLinkItem.Interpolation = AnimSeqExportOption->Interpolation;
	// 							// LevelAnimLinkItem.CurveInterpolation = AnimSeqExportOption->CurveInterpolation;
	//
	// 							break;
	// 						}
	// 					}
	// 				}
	// 				else
	// 				{
	// 					LevelAnimLink = NewObject<ULevelSequenceAnimSequenceLink>(
	// 						LevelSequence, NAME_None, RF_Public | RF_Transactional);
	// 				}
	// 				if (bAddItem == true)
	// 				{
	// 					FLevelSequenceAnimSequenceLinkItem LevelAnimLinkItem;
	// 					LevelAnimLinkItem.SkelTrackGuid = Binding;
	// 					LevelAnimLinkItem.PathToAnimSequence = FSoftObjectPath(AnimSequence);
	// 					// LevelAnimLinkItem.bExportMorphTargets = AnimSeqExportOption->bExportMorphTargets;
	// 					// LevelAnimLinkItem.bExportAttributeCurves = AnimSeqExportOption->bExportAttributeCurves;
	// 					// LevelAnimLinkItem.bExportMaterialCurves = AnimSeqExportOption->bExportMaterialCurves;
	// 					// LevelAnimLinkItem.bExportTransforms = AnimSeqExportOption->bExportTransforms;
	// 					// LevelAnimLinkItem.bRecordInWorldSpace = AnimSeqExportOption->bRecordInWorldSpace;
	// 					// LevelAnimLinkItem.bEvaluateAllSkeletalMeshComponents = AnimSeqExportOption->
	// 					// 	bEvaluateAllSkeletalMeshComponents;
	// 					// LevelAnimLinkItem.Interpolation = AnimSeqExportOption->Interpolation;
	// 					// LevelAnimLinkItem.CurveInterpolation = AnimSeqExportOption->CurveInterpolation;
	// 					//
	// 					LevelAnimLink->AnimSequenceLinks.Add(LevelAnimLinkItem);
	// 					AssetUserDataInterface->AddAssetUserData(LevelAnimLink);
	// 				}
	// 			}
	// 		}
	// 	}
	// 	// if it contains error, warn them
	// 	// if (bResult)
	// 	// {
	// 		FText NotificationText;
	// 		if (NewAssets.Num() == 1)
	// 		{
	// 			NotificationText = FText::Format(
	// 				LOCTEXT("NumAnimSequenceAssetsCreated", "{0} Anim Sequence  assets created."), NewAssets.Num());
	// 		}
	// 		else
	// 		{
	// 			NotificationText = FText::Format(
	// 				LOCTEXT("AnimSequenceAssetsCreated", "Anim Sequence asset created: '{0}'."),
	// 				FText::FromString(NewAssets[0]->GetName()));
	// 		}
	//
	// 		FNotificationInfo Info(NotificationText);
	// 		Info.ExpireDuration = 8.0f;
	// 		Info.bUseLargeFont = false;
	// 		Info.Hyperlink = FSimpleDelegate::CreateLambda([NewAssets]()
	// 		{
	// 			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(NewAssets);
	// 		});
	// 		Info.HyperlinkText = FText::Format(
	// 			LOCTEXT("OpenNewPoseAssetHyperlink", "Open {0}"), FText::FromString(NewAssets[0]->GetName()));
	//
	// 		TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	// 		if (Notification.IsValid())
	// 		{
	// 			Notification->SetCompletionState(SNotificationItem::CS_Success);
	// 		}
	// 	// }
	// 	// else
	// 	// {
	// 	// 	FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FailedToCreateAsset", "Failed to create asset"));
	// 	// }
	// }
	return true;
}


void FMontageGraphDetails::OpenLinkedAnimation(UMontageGraphEdNodeMontage* MontageEdNode)
{
}


void FMontageGraphDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Animation",
	                                                              FText::FromString("Animation"));

	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

	UMontageGraphEdNodeMontage* MontageEdNode = nullptr;
	//Check whether the AnimNode already has a montage
	bool bNoMontage = true;
	if (CustomizedObjects.Num() > 0)
	{
		MontageEdNode = Cast<UMontageGraphEdNodeMontage>(CustomizedObjects[0].Get());

		if (MontageEdNode && MontageEdNode->RuntimeNode)
		{
			UMontageGraphNode_Animation* AnimNode = Cast<UMontageGraphNode_Animation>(MontageEdNode->RuntimeNode);
			if (AnimNode)
			{
				bNoMontage = !IsValid(AnimNode->AnimationMontage);
			}
			if (UMontageGraphEdGraph* Graph = Cast<UMontageGraphEdGraph>(MontageEdNode->GetGraph()))
			{
				GraphBeingEdited = Graph->GetHBMontageGraphModel();
			}
		}
	}

	if (MontageEdNode != nullptr && !IsValid(GraphBeingEdited->ControlRigClass))
	{
		return;
	}

	//If Not add option to create a linked montage using control rig
	TArray<TSharedRef<IPropertyHandle>> AllProperties;
	Category.GetDefaultProperties(AllProperties);

	for (auto& Property : AllProperties)
	{
		Category.AddProperty(Property); // Copy existing properties so our btn is at the bottom
	}

	if (MontageEdNode->bHasLinkedAnimation)
	{
		FSimpleDelegate OnClickDelegate = FSimpleDelegate::CreateLambda([this, MontageEdNode]()
		{
			OpenLinkedAnimation(MontageEdNode);
		});
		Category.AddCustomRow(FText::FromString("MontageGraphFooter"))
		        .WholeRowWidget
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(FMargin(.0f, 10.f, .0f, 5.f))
			[
				SNew(SButton)
				.ContentPadding(FMargin(.0f, 10.f, .0f, 10.f))
				.HAlign(HAlign_Center)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
				.OnPressed(OnClickDelegate)
				.Content()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("Sequencer.AllowSequencerEditsOnly"))
					]

					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .Padding(FMargin(5.0f, 0.0f))
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString("Edit Linked Animation in Sequencer"))
						.TextStyle(FCoreStyle::Get(), "NormalText")
					]
				]
			]
		];
	}
	else
	{
		FSimpleDelegate OnClickDelegate = FSimpleDelegate::CreateLambda([this, MontageEdNode, &DetailBuilder]()
		{
			CreateLinkedControlRigAnimationForNode(MontageEdNode);

			MontageEdNode->bHasLinkedAnimation = true;
			MontageEdNode->Modify(true);
			DetailBuilder.ForceRefreshDetails();
		});

		Category.AddCustomRow(FText::FromString("MontageGraphFooter"))
		        .WholeRowWidget
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(FMargin(.0f, 10.f, .0f, 5.f))
			[
				SNew(SButton)
    		.ContentPadding(FMargin(.0f, 10.f, .0f, 10.f))
    		.HAlign(HAlign_Center)
    		.ButtonStyle(FAppStyle::Get(), "FlatButton.Primary")
    		.OnPressed(OnClickDelegate)
    		.Content()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					[
						SNew(SImage)
    				.Image(FCoreStyle::Get().GetBrush("Plus"))
    				.ColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f))
					]

					+ SHorizontalBox::Slot()
					  .AutoWidth()
					  .VAlign(VAlign_Center)
					  .Padding(FMargin(5.0f, 0.0f))
					[
						SNew(STextBlock)
    				.Justification(ETextJustify::Center)
    				.Text(FText::FromString("Create Linked Control Rig Animation"))
    				.TextStyle(FCoreStyle::Get(), "NormalText")
					]
				]
			]
		];
	}
}
#undef LOCTEXT_NAMESPACE
