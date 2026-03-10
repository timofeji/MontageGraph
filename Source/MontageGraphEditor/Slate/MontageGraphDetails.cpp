#include "MontageGraphDetails.h"

#include <Graph/EdNodes/MGEdNode_Montage.h>
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorStyleSet.h"
#include "PersonaTabs.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "Graph/MontageEdGraph.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "FMontageGraphDetails"


// FScopedTransaction Transaction(FText::FromString(NewKeys[0]));
// Modify();
// GetGraph()->NotifyNodeChanged(this);
void FMontageGraphDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	if (UMGEdNode_Montage* MontageEdNode = Cast<UMGEdNode_Montage>(Objects[0]))
	{
		TArray<UObject*> AllSections;
		TMap<UObject*, UDopeSheetTrackBase*> SectionToTrackMap;
		auto             CollectSectionsRecursive = [&](UDopeSheetTrackBase* Track, auto&& CollectRef) -> void {
			if (!Track)
			{
				return;
			}

			for (UDopeSheetTrackSection* Section : Track->Sections)
			{
				if (Section)
				{
					SectionToTrackMap.Add(Section, Track);
					AllSections.Add(Section);
				}
			}
			for (UDopeSheetTrackBase* SubTrack : Track->SubTracks)
			{
				CollectRef(SubTrack, CollectRef);
			}
		};

		for (UDopeSheetTrackBase* Track : MontageEdNode->MontageTracks)
		{
			CollectSectionsRecursive(Track, CollectSectionsRecursive);
		}

		// Create a collapsible section header
		IDetailCategoryBuilder& TracksCategory = DetailBuilder.EditCategory(
			FName("MontageTracks"),
			LOCTEXT("Montage Tracks", "Montage Tracks"),
			ECategoryPriority::Important);

		TracksCategory.HeaderContent(
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.2f, 0.4f, 0.8f, 1.0f))
			.Padding(FMargin(0.0f, 8.0f))
			[
				SNew(STextBlock)
				.Margin(FMargin(16.0f, 0.0f))
				.Text(FText::FromString(MontageEdNode->MontageDisplayName))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Left)
			]
		  , true);

		if (AllSections.Num() > 0)
		{
			for (int32 i = 0; i < AllSections.Num(); ++i)
			{
				UDopeSheetTrackSection* Section = Cast<UDopeSheetTrackSection>(AllSections[i]);
				if (Section)
				{
					FString SectionName = Section->GetClass()->GetName();
					SectionName = SectionName.Replace(TEXT("MontageTrackSection_"), TEXT(""));

					TracksCategory.AddCustomRow(FText::FromString("Section Header"))
								  .WholeRowWidget
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						.BorderBackgroundColor(SectionToTrackMap[Section]->GetTrackColor())
						.Padding(FMargin(8.0f, 6.0f))
						[
							SNew(SHorizontalBox)

							// Section name and type
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(FText::FromString(SectionName))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
									.ColorAndOpacity(FLinearColor::White)
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString::Printf(TEXT("Type: %s"), *Section->GetClass()->GetName())))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
									.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 0.8f))
								]
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(5.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SButton)
								.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
								.ToolTipText(FText::FromString(FString::Printf(TEXT("Delete %s"), *SectionName)))
								.ContentPadding(FMargin(4.0f))
								.OnClicked_Lambda([this, Section, SectionToTrackMap]() -> FReply {
									const FScopedTransaction Transaction(NSLOCTEXT("MontageNodeDetails", "RemoveMontageTrackSection_Transaction",
										"Remove MontageTrack Section"));

									auto TrackObj = SectionToTrackMap[Section];
									Section->MarkAsGarbage();
									TrackObj->Modify(true);
									TrackObj->Sections.Remove(Section);
									TrackObj->OnTrackPropertiesChanged.Broadcast();

                                	return FReply::Handled();
                                })
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString("x"))
                                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                                    .ColorAndOpacity(FLinearColor::White)
                                    .Justification(ETextJustify::Center)
                                ]
                            ]
                        ]
                    ];

					FDetailsViewArgs DetailsArgs(
						false,
						false,
						false,
						FDetailsViewArgs::HideNameArea,
						false,
						nullptr,
						false,
						FName()
						);

					DetailsArgs.bShowOptions                  = false;
					DetailsArgs.bShowModifiedPropertiesOption = false;
					DetailsArgs.bShowKeyablePropertiesOption  = false;
					DetailsArgs.bShowAnimatedPropertiesOption = false;
					DetailsArgs.bAllowFavoriteSystem          = false;

					TSharedPtr<IDetailsView> SectionDetailsView = PropertyModule.CreateDetailView(DetailsArgs);

					if (SectionDetailsView.IsValid())
					{
						SectionDetailsView->SetObject(Section);
						TracksCategory.AddCustomRow(FText::FromString("Properties"))
									   .WholeRowContent()
									   .HAlign(HAlign_Fill)
						[
							SNew(SBox)
							.MinDesiredHeight(100.0f)
							[
								SectionDetailsView.ToSharedRef()
							]
						];
					}

				}
			}

		}
	}
}

void FMontageGraphDetails ::SortCustomDetailsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap)
{
	// (*AllCategoryMap.Find(FName("Editor")))->SetSortOrder(1000);
}



#undef LOCTEXT_NAMESPACE
