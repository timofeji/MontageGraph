#include "MontageGraphDetails.h"

#include <Graph/Nodes/MontageGraphEdNodeMontage.h>

#include "AnimationEditorUtils.h"
#include "AssetToolsModule.h"
#include "ControlRigObjectBinding.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "FileHelpers.h"
#include "LevelSequenceEditor/Public/ILevelSequenceEditorToolkit.h"
#include "Editor/Sequencer/Public/ISequencer.h"
#include "LevelEditorViewport.h"
#include "LevelSequence.h"
#include "MontageGraphEditorLog.h"
#include "MovieScene.h"
#include "SequencerChannelTraits.h"
#include "SequencerTools.h"
#include "Animation/SkeletalMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Exporters/AnimSeqExportOption.h"
#include "Factories/AnimMontageFactory.h"
#include "Factories/AnimSequenceFactory.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Graph/MontageGraphEdGraph.h"
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

			if (UObject* SpawnedMesh = Sequencer->FindSpawnedObjectOrTemplate(ActorTrackGuid))
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

			TArray<UPackage*> PackagesToSave;
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


				FMovieSceneBindingProxy Binding(ActorTrackGuid, LevelSequence);
				UAnimSeqExportOption* ExportOptions = NewObject<UAnimSeqExportOption>();
				USequencerToolsFunctionLibrary::LinkAnimSequence(LevelSequence, NewAnimSequence, ExportOptions,
				                                                 Binding);

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

				FFormatNamedArguments Args;
				Args.Add(TEXT("AnimSequenceName"), FText::FromString(NewAnimSequence->GetName()));
				Args.Add(TEXT("AnimMontageName"), FText::FromString(NewAnimMontage->GetName()));
				FNotificationInfo Info(FText::Format(LOCTEXT("LinkedAnimCreatedNotification",
				                                             "Sequence: \"{AnimSequenceName}\" linked to Montage:  \"{AnimMontageName}\""), Args));
				Info.ExpireDuration = 3.0f;
				Info.bUseLargeFont = false;
				TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
				if (Notification.IsValid())
				{
					Notification->SetCompletionState(SNotificationItem::CS_Success);
				}


				PackagesToSave.Add(NewAnimSequence->GetPackage());
				PackagesToSave.Add(NewAnimMontage->GetPackage());
			}


			//Instantly Save
			PackagesToSave.Add(LevelSequencePackage);
			FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false);
		}
	}
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
				GraphBeingEdited = Graph->GetMontageGraphModel();
			}
		}
	}

	if (MontageEdNode == nullptr || !IsValid(GraphBeingEdited->ControlRigClass))
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

	if (MontageEdNode->HasLinkedMontage())
	{
		FSimpleDelegate OnClickDelegate = FSimpleDelegate::CreateLambda([this, MontageEdNode]()
		{
			MontageEdNode->OpenLinkedAnimation();
		});
		Category.AddCustomRow(FText::FromString("MontageGraphFooter"))
		        .WholeRowWidget
		[
			// SNew(SOverlay)
			// + SOverlay::Slot()
			// .Padding(FMargin(.0f, 10.f, .0f, 5.f))
			// [
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				  .FillWidth(1.0f)
				  .VAlign(VAlign_Center)
				[
					SNew(SButton)
												.ContentPadding(FMargin(.0f, 10.f, .0f, 10.f))
												.HAlign(HAlign_Fill)
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
						  .FillWidth(1.0f)
						  .VAlign(VAlign_Center)
						  .Padding(FMargin(3.0f, 0.0f))
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString("Edit Sequence"))
							.TextStyle(FAppStyle::Get(), "FlatButton.DefaultTextStyle")
						]
					]
				]
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .VAlign(VAlign_Fill)
				  .Padding(1.0f, 0.f)
				[
					SNew(SSeparator)
					.Orientation(Orient_Vertical)
					.Thickness(3.0f)
				]
				+ SHorizontalBox::Slot()
				  .AutoWidth()
				  .VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.ContentPadding(FMargin(.0f, 10.f, .0f, 10.f))
					.HAlign(HAlign_Right)
					.ButtonStyle(FAppStyle::Get(), "FlatButton.Dark")
					.OnPressed(OnClickDelegate)
					.Content()
					[
						SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("SystemWideCommands.FindInContentBrowser.Small"))
					]
				]
			// ]
		];
	}
	else
	{
		FSimpleDelegate OnClickDelegate = FSimpleDelegate::CreateLambda([this, MontageEdNode, &DetailBuilder]()
		{
			CreateLinkedControlRigAnimationForNode(MontageEdNode);

			MontageEdNode->Modify(true);
			DetailBuilder.ForceRefreshDetails();
		});

		Category.AddCustomRow(FText::FromString("MontageGraphFooter"))
		        .WholeRowWidget
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
    				.Text(FText::FromString("Create Linked Animation Assets"))
					.TextStyle(FAppStyle::Get(), "FlatButton.DefaultTextStyle")
					]
				]
		];
	}
}
#undef LOCTEXT_NAMESPACE
