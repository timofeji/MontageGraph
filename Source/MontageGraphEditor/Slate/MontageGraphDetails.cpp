#include "MontageGraphDetails.h"

#include <Graph/Nodes/MontageGraphEdNodeMontage.h>

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "MontageGraph/MontageGraphNode_Animation.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

void FMontageGraphDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Animation",
	                                                                FText::FromString("Animation"));

	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

	//Check whether the AnimNode already has a montage
	bool bNoMontage = true; 
	if (CustomizedObjects.Num() > 0)
	{
		auto MontageEdNode = Cast<UMontageGraphEdNodeMontage>(CustomizedObjects[0].Get());
		if (MontageEdNode && MontageEdNode->RuntimeNode)
		{
			UMontageGraphNode_Animation* AnimNode = Cast<UMontageGraphNode_Animation>(MontageEdNode->RuntimeNode);
			if(AnimNode)
			{
				bNoMontage = !IsValid(AnimNode->AnimationMontage);
			}
		}
	}
	
	if(!bNoMontage)
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
	
	Category.AddCustomRow(FText::FromString("MontageGraphFooter"))
	          .WholeRowWidget
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		.Padding(FMargin(.0f, 15.f, .0f, 5.f))
		[
		SNew(SButton)
		.ContentPadding(FMargin(.0f, 10.f, .0f, 10.f))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "FlatButton.Primary")
		.OnClicked_Lambda([]() -> FReply
		             {
			             return FReply::Handled();
		             })
		.Content()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FCoreStyle::Get().GetBrush("Plus"))
				.ColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)) // Green color for the plus icon
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
