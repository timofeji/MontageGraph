#include "MontageGraphDetails.h"

#include <Graph/Nodes/MontageGraphEdNodeMontage.h>

#include "AssetToolsModule.h"
#include "ControlRigObjectBinding.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "LevelSequenceEditor/Public/ILevelSequenceEditorToolkit.h"
#include "Editor/Sequencer/Public/ISequencer.h"
#include "LevelEditorViewport.h"
#include "LevelSequence.h"
#include "MontageGraphEditorLog.h"
#include "MovieScene.h"
#include "Animation/SkeletalMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Graph/MontageGraphEdGraph.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphNode_Animation.h"
#include "Sequencer/MovieSceneControlRigParameterTrack.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"


//Largely copied from ControlRigEditorModule.cpp
void FMontageGraphDetails::CreateLinkedControlRigMontageForNode(UMontageGraphEdNodeMontage* AnimNode) const
{
	UWorld* World = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;

	if (!World)
	{
		MG_ERROR(Verbose, TEXT("Error: Attempting to create a linked control rig montage but no world is detected"));
		return;
	}

	const FString PrefixStr = GraphBeingEdited->GraphOutputPrefix.IsEmpty()
		                          ? TEXT("Driving")
		                          : FString::Printf(TEXT("Driving_%s"), *GraphBeingEdited->GraphOutputPrefix);

	const FString SequenceName = FString::Printf(TEXT("%s_%s"),
	                                             *PrefixStr,
	                                             *AnimNode->AnimationName);

	const FString PackagePath = GraphBeingEdited->GetOutermost()->GetName() + GraphBeingEdited->GraphOutputBaseDir;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString UniquePackageName;
	FString UniqueAssetName;
	AssetToolsModule.Get().CreateUniqueAssetName(PackagePath / SequenceName, TEXT(""), UniquePackageName,
	                                             UniqueAssetName);

	UPackage* Package = CreatePackage(*UniquePackageName);
	ULevelSequence* LevelSequence = NewObject<ULevelSequence>(Package, *UniqueAssetName, RF_Public | RF_Standalone);

	FAssetRegistryModule::AssetCreated(LevelSequence);

	LevelSequence->Initialize(); //creates movie scene
	LevelSequence->MarkPackageDirty();
	UMovieScene* MovieScene = LevelSequence->GetMovieScene();


	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(LevelSequence);


	IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(
		LevelSequence, false);
	ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(AssetEditor);
	TWeakPtr<ISequencer> WeakSequencer = LevelSequenceEditor ? LevelSequenceEditor->GetSequencer() : nullptr;


	if (WeakSequencer.IsValid())
	{
		ASkeletalMeshActor* MeshActor = World->SpawnActor<ASkeletalMeshActor>(
			ASkeletalMeshActor::StaticClass(), FTransform::Identity);
		MeshActor->SetActorLabel(SequenceName);

		FString StringName = MeshActor->GetActorLabel();
		FString AnimName = AnimNode->AnimationName;
		StringName = StringName + FString(TEXT(" --> ")) + AnimName;
		MeshActor->SetActorLabel(StringName);


		// if (SkelMesh)
		// {
		// 	MeshActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkelMesh);
		// }
		MeshActor->RegisterAllComponents();
		TArray<TWeakObjectPtr<AActor>> ActorsToAdd;
		ActorsToAdd.Add(MeshActor);
		TArray<FGuid> ActorTracks = WeakSequencer.Pin()->AddActors(ActorsToAdd, false);
		FGuid ActorTrackGuid = ActorTracks[0];

		// By default, convert this to a spawnable and delete the existing actor. If for some reason, 
		// the spawnable couldn't be generated, use the existing actor as a possessable (this could 
		// eventually be an option)
		TArray<FGuid> SpawnableGuids = WeakSequencer.Pin()->ConvertToSpawnable(ActorTrackGuid);
		if (SpawnableGuids.Num())
		{
			ActorTrackGuid = SpawnableGuids[0];

			UObject* SpawnedMesh = WeakSequencer.Pin()->FindSpawnedObjectOrTemplate(ActorTrackGuid);

			if (SpawnedMesh)
			{
				GCurrentLevelEditingViewportClient->GetWorld()->EditorDestroyActor(MeshActor, true);
				MeshActor = Cast<ASkeletalMeshActor>(SpawnedMesh);
				// if (SkelMesh)
				// {
				// 	MeshActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkelMesh);
				// }
				MeshActor->RegisterAllComponents();
			}
		}

		//Delete binding from default animating rig
		//if we have skel mesh component binding we can just delete that
		FGuid CompGuid = WeakSequencer.Pin()->FindObjectId(*(MeshActor->GetSkeletalMeshComponent()),
		                                                   WeakSequencer.Pin()->GetFocusedTemplateID());
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


			WeakSequencer.Pin()->
			              NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemsChanged);


			Track->Modify();
			UMovieSceneSection* NewSection = Track->CreateControlRigSection(0, ControlRig, true);
			//mz todo need to have multiple rigs with same class
			Track->SetTrackName(FName(*ObjectName));
			Track->SetDisplayName(FText::FromString(ObjectName));
			UMovieSceneControlRigParameterSection* ParamSection = Cast<UMovieSceneControlRigParameterSection>(
				NewSection);
				
				
			WeakSequencer.Pin()->SelectTrack(Track);
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
				GraphBeingEdited = Graph->GetHBMontageGraphModel();
			}
		}
	}

	if (!IsValid(GraphBeingEdited->ControlRigClass) && MontageEdNode != nullptr)
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

	FSimpleDelegate OnClickDelegate = FSimpleDelegate::CreateLambda([this, MontageEdNode]()
	{
		CreateLinkedControlRigMontageForNode(MontageEdNode);

		GraphBeingEdited->Modify(true);
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
