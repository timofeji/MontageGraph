#include "MontageGraphDetails.h"

#include <Graph/Nodes/MontageGraphEdNodeMontage.h>

#include "AssetToolsModule.h"
#include "ControlRig/Public/ControlRig.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "LevelEditorViewport.h"
#include "LevelSequence.h"
#include "MontageGraphEditorLog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Graph/MontageGraphEdGraph.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/MontageGraphNode_Animation.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"


void FMontageGraphDetails::CreateLinkedControlRigMontageForNode(UMontageGraphEdNodeMontage* AnimNode) const
{
	UWorld* World = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;

	if (!World)
	{
		MG_ERROR(Verbose, TEXT("Error: Attempting to create a linked control rig montage but no world is detected"));
		return;
	}

	const FString SequenceName = FString::Printf(TEXT("Driving_%s_%s"),
	                                             *GraphBeingEdited->GraphOutputPrefix,
	                                             *AnimNode->AnimationName);
	
	const FString PackagePath = GraphBeingEdited->GraphOutputBaseDir.Path;

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

	if (!bNoMontage && MontageEdNode != nullptr)
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
