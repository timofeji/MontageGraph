#include "MontageTrack_Sequences.h"

#include "AssetSelection.h"
#include "AssetToolsModule.h"
#include "MontageGraphEditorLog.h"
#include "MontageGraphEditorStyle.h"
#include "MontageTrack_BlendLinks.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Slate/SAnimMontageSlotPicker.h"

namespace MGDopeSheetBrushes
{
	FColor SequenceSectionColor = FColor(80, 123, 72, 255);

	static const FSlateBrush* SectionBodyBrush         = nullptr;
	static const FSlateBrush* SectionBodyBrush_Hovered = nullptr;
	static const FSlateBrush* SectionBorderBrush       = nullptr;
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "MontageTrack_Sequences"
TSharedPtr<SWidget> FDopeSheetSequenceSectionDragDrop::GetDefaultDecorator() const
{
	return SectionBeingDragged.Pin();
}

void FDopeSheetSequenceSectionDragDrop::Construct()
{
	FDragDropOperation::Construct();

	CursorDecoratorWindow->SetOpacity(.25f);
}

void FDopeSheetSequenceSectionDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	// if ( bDropWasHandled == false )
	// {
	// 	if (SectionBeingDragged.IsValid())
	// 	{
	// 		SectionBeingDragged.Pin()->OnDropCancelled(MouseEvent);
	// 	}
	// }
	//

	if (OwningTrack.IsValid())
	{
		OwningTrack.Pin()->OnRearrangeDrop();
	}


	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}


void FDopeSheetSequenceSectionDragDrop::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	FSlateRect BoundingRect = OwningTrack.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
	FSlateRect SectionRect  = SectionBeingDragged.Pin()->GetCachedGeometry().GetLayoutBoundingRect();


	FVector2D MousePos = DragDropEvent.GetScreenSpacePosition();
	FVector2D SectionDerotatowPos;
	SectionDerotatowPos.X = FMath::Clamp((MousePos + Offset).X,
	                                     BoundingRect.Left,
	                                     BoundingRect.Right - SectionRect.GetSize().X);
	SectionDerotatowPos.Y = StartingScreenPos.Y;
	CursorDecoratorWindow->MoveWindowTo(SectionDerotatowPos);

	OwningTrack.Pin()->PreviewRearrange(MousePos);
}

TSharedRef<FDopeSheetSequenceSectionDragDrop> FDopeSheetSequenceSectionDragDrop::New(
	TSharedRef<SMGSequenceTrack> ParentTrack,
	TSharedRef<SWidget>          SectionWidget, const FVector2D& CursorPosition, const FVector2D& ScreenPositionOfNode)
{
	TSharedRef<FDopeSheetSequenceSectionDragDrop> Operation = MakeShareable(new FDopeSheetSequenceSectionDragDrop);

	Operation->OwningTrack         = ParentTrack;
	Operation->SectionBeingDragged = SectionWidget;
	Operation->Offset              = ScreenPositionOfNode - CursorPosition;
	Operation->StartingScreenPos   = ScreenPositionOfNode;

	Operation->Construct();


	return Operation;
}

/** Constructs this widget with InArgs */
void SMGSequenceTrackSection::Construct(const FArguments& InArgs)
{
	MGDopeSheetBrushes::SectionBodyBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Body");
	MGDopeSheetBrushes::SectionBodyBrush_Hovered = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Body.Hovered");
	MGDopeSheetBrushes::SectionBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Border");


	SequencePtr = InArgs._Sequence;
	SetCursor(EMouseCursor::GrabHand);
	ChildSlot[
		SNew(SBorder)
		.BorderImage_Lambda([] { return MGDopeSheetBrushes::SectionBorderBrush; })
		.BorderBackgroundColor(MGDopeSheetBrushes::SequenceSectionColor)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.ColorAndOpacity(MGDopeSheetBrushes::SequenceSectionColor)
				.Image_Lambda([&]()

				{
					return bIsHovered
						       ? MGDopeSheetBrushes::SectionBodyBrush_Hovered
						       : MGDopeSheetBrushes::SectionBodyBrush;
				})
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(5)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InArgs._Sequence->GetName()))
			]
		]
	];
}


void SMGSequenceTrackSection::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsHovered = true;
};

void SMGSequenceTrackSection::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	bIsHovered = false;
};

void SMGSequenceTrackSection::Select()
{
};

FReply SMGSequenceTrackSection::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && SequencePtr)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(
			TEXT("AssetTools"));
		TWeakPtr<IAssetTypeActions> AssetTypeActions = AssetToolsModule.Get().GetAssetTypeActionsForClass(
			UAnimSequence::StaticClass());
		if (AssetTypeActions.IsValid())
		{
			const TArray<UObject*> AssetsToOpen{SequencePtr};
			AssetTypeActions.Pin()->OpenAssetEditor(AssetsToOpen);
		}
	}
	return FReply::Handled();
};


FVector2D SMGSequenceTrackSection::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return GetCachedGeometry().Size;
}

void SMGSequenceTrack::Construct(const FArguments&                           InArgs,
                                 const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr)
{
	TrackModel = TrackModelPtr;

	if (TrackModel.IsValid() && TrackModel->ObjPtr)
	{
		TrackModel->ObjPtr->OnTrackPropertiesChanged.AddRaw(
			this, &SMGSequenceTrack::RegenerateSequenceSections);
	}


	RegenerateSequenceSections();
}

SMGSequenceTrack::~SMGSequenceTrack()
{
	if (TrackModel.IsValid() && TrackModel->ObjPtr)
	{
		TrackModel->ObjPtr->OnTrackPropertiesChanged.RemoveAll(this);
	}
}


FReply SMGSequenceTrack::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FVector2D ScreenCursorPos = MouseEvent.GetScreenSpacePosition();
		FVector2D SectionOffset   = FVector2D(SelectedSectionWidget->GetCachedGeometry().GetAbsolutePosition());

		bPreviewSectionRearrange = true;

		return FReply::Handled().BeginDragDrop(
			FDopeSheetSequenceSectionDragDrop::New(SharedThis(this), SelectedSectionWidget, ScreenCursorPos,
			                                       SectionOffset)).ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

void SMGSequenceTrack::PreviewRearrange(FVector2D ScreenCursorPos)
{
	RearrangeIndices.Reset();
	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		RearrangeIndices.Add(ChildIndex);
	}

	//Find Section Being Dragged Over
	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		const TSharedRef<SWidget> SectionWidget = TrackSectionWidgets[ChildIndex];

		const EVisibility ChildVisibility = SectionWidget->GetVisibility();
		if (ChildVisibility != EVisibility::Visible) { continue; }

		FSlateRect SectionRect = SectionWidget->GetCachedGeometry().GetRenderBoundingRect();

		if (SectionRect.Left < ScreenCursorPos.X
			&& SectionRect.Right > ScreenCursorPos.X
			&& SelectedSectionIndex != ChildIndex)
		{
			RearrangeIndices.Swap(ChildIndex, SelectedSectionIndex);
			break;
		}
	}
}

FReply SMGSequenceTrack::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();


	if (Operation->IsOfType<FDopeSheetSequenceSectionDragDrop>() && bPreviewSectionRearrange)
	{
	}


	if (Operation->IsOfType<FAssetDragDropOp>())
	{
		if ((TrackModel != nullptr) && (TrackModel->ObjPtr != nullptr))
		{
			TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(Operation);
			FText                        TooltipText;
			if (AssetOp->HasAssets())
			{
				const TArray<FAssetData>& HoveredAssetData = AssetOp->GetAssets();
				FText                     AssetReferenceFilterFailureReason;
				if (PassesAssetReferenceFilter(HoveredAssetData))
				{
					FString TooltipTextString;
					// TrackModel->GetAssetsHoverMessage(HoveredAssetData, TrackModel, /*out*/
					//                                                   TooltipTextString, /*out*/ bOkIcon);
					TooltipText       = FText::FromString(TooltipTextString);
					bIsValidAssetDrop = true;
				}
				else
				{
					TooltipText       = AssetReferenceFilterFailureReason;
					bIsValidAssetDrop = false;
				}
			}
			const FSlateBrush* TooltipIcon = bIsValidAssetDrop
				                                 ? FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK"))
				                                 : FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
			AssetOp->SetToolTip(TooltipText, TooltipIcon);
		}
		return FReply::Handled();
	}
	else
	{
		return SDopeSheetTrackTimeline::OnDragOver(MyGeometry, DragDropEvent);
	}
}

bool SMGSequenceTrack::PassesAssetReferenceFilter(const TArray<FAssetData>& Array)
{
	bool bIsValid = true;
	for (auto Asset : Array)
	{
		if (!Asset.IsInstanceOf(UAnimSequence::StaticClass()))
		{
			bIsValid = false;
			break;
		}
	}

	return bIsValid;
}


void SMGSequenceTrack::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	bIsValidAssetDrop = false;
}

void SMGSequenceTrack::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	bIsValidAssetDrop = false;
}

FReply SMGSequenceTrack::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if ((TrackModel != nullptr) && (TrackModel->ObjPtr != nullptr))
	{
		TArray<FAssetData> DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(DragDropEvent);

		if (DroppedAssetData.Num() > 0)
		{
			if (PassesAssetReferenceFilter(DroppedAssetData))
			{
				TrackModel->ObjPtr->DroppedAssetsOnTrack(DroppedAssetData);
			}
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

int32 SMGSequenceTrack::OnPaint(const FPaintArgs&   Args, const FGeometry&                  AllottedGeometry,
                                const FSlateRect&   MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                int32               InLayerId,
                                const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	static const FSlateBrush* PreviewBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Preview");
	static const FSlateBrush* SectionBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Border");


	if (bPreviewSectionRearrange)
	{
		const float AllottedWidth     = static_cast<float>(AllottedGeometry.GetLocalSize().X);
		const float AllottedHeight    = static_cast<float>(AllottedGeometry.GetLocalSize().Y);
		float       AccumulatedOffset = TrackModel->Controller->EditableRect.Left;
		for (int32 i = 0; i < RearrangeIndices.Num(); ++i)
		{
			int32 Index   = RearrangeIndices[i];
			auto  Section = TrackModel->SectionModels[Index];

			const float ViewedTimeCoefficient = Section.GetTimeLength() / TrackModel->Controller->TimeDurationInView;
			const float SectionWidth          = AllottedWidth * ViewedTimeCoefficient;

			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				InLayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2D(AccumulatedOffset, 0),
				                                 FVector2D(SectionWidth, AllottedHeight)),
				Index == SelectedSectionIndex ? PreviewBorderBrush : SectionBorderBrush,
				ESlateDrawEffect::None,
				Index == SelectedSectionIndex ? FColor(168, 255, 153, 255) : FColor(110, 110, 110, 255)
			);


			AccumulatedOffset += SectionWidth;
		}


		return InLayerId;
	}


	int32 LayerId = SDopeSheetTrackTimeline::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, InLayerId,
	                                                 InWidgetStyle, bParentEnabled);


	if (bIsValidAssetDrop)
	{
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			SectionBorderBrush,
			ESlateDrawEffect::None,
			FColor(80, 123, 72, 255)
		);
	}

	return LayerId;
}

void SMGSequenceTrack::OnRearrangeDrop()
{
	if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(TrackModel->ObjPtr))
	{
		TArray<UDopeSheetTrackSection> Temp;

		//
		// for (int32 i = 0; i < RearrangeIndices.Num(); ++i)
		// {
		// 	int32 Index = RearrangeIndices[i];
		//
		// 	if (UMontageTrackSection_Sequences* MontagePart = Cast<UMontageTrackSection_Sequences>(
		// 		SequencesTrack->Sections[i]))
		// 	{
		// 		Temp.Add(SequencesTrack->Sections[Index]);
		// 	}
		// }

	}
	bPreviewSectionRearrange = false;
	TrackModel->ObjPtr->OnTrackPropertiesChanged.Broadcast();
}



void SMGSequenceTrack::OnArrangeChildren(const FGeometry&   AllottedGeometry,
                                                FArrangedChildren& ArrangedChildren) const
{
	if (!TrackModel)
	{
		return;
	}

	float AccumulatedOffset = TrackModel->Controller->EditableRect.Left;
	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		const TSharedRef<SWidget> ChildWidget = TrackSectionWidgets[ChildIndex];

		const EVisibility ChildVisibility = ChildWidget->GetVisibility();
		if (!ArrangedChildren.Accepts(ChildVisibility))
		{
			continue;
		}

		const float AllottedWidth = static_cast<float>(AllottedGeometry.GetLocalSize().X);
		const float AllottedHeight = static_cast<float>(AllottedGeometry.GetLocalSize().Y);
			
		auto Section = TrackModel->SectionModels[ChildIndex];
		
		const float ViewedTimeCoefficient = Section.GetTimeLength() / TrackModel->Controller->TimeDurationInView;
		const float SectionWidth = AllottedWidth * ViewedTimeCoefficient;
		

		ArrangedChildren.AddWidget( AllottedGeometry.MakeChild(
			                           ChildWidget,
			                           FVector2D(AccumulatedOffset, 0),
			                           FVector2D(SectionWidth, AllottedHeight)));
		
		
		AccumulatedOffset += SectionWidth;
	}
}

void SMGSequenceTrack::RegenerateSequenceSections()
{
	TrackSectionWidgets.Reset();


	if ((TrackModel != nullptr) && (TrackModel->ObjPtr))
	{
		if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(TrackModel->ObjPtr))
		{
			const int NumOfSections = SequencesTrack->Sections.Num();
			TrackModel->SectionModels.Reset(NumOfSections);

			float AccumulatedTime = 0.f;
			for (int i = 0; i < NumOfSections; i++)
			{
				if (UMontageTrackSection_Sequences* MontageSection =
					Cast<UMontageTrackSection_Sequences>(SequencesTrack->Sections[i]))
				{
					UAnimSequence* AnimSequence = MontageSection->AnimSequence;

					const float StartTime = AccumulatedTime;
					AccumulatedTime += AnimSequence->GetPlayLength();
					TrackModel->SectionModels.Add(FDopeSheetSectionViewModel(StartTime, AccumulatedTime));

					TrackSectionWidgets.Add(SNew(SMGSequenceTrackSection).Sequence(AnimSequence));
				}
			}
		}
	}
}


UMontageTrack_Sequences::UMontageTrack_Sequences(const FObjectInitializer& ObjectInitializer)
{
	UMontageTrack_BlendLinks* BlendLinksTrack =
		ObjectInitializer.CreateDefaultSubobject<UMontageTrack_BlendLinks>(this,TEXT("MontageTrack_BlendLinks"));

	SubTracks.Add(BlendLinksTrack);

	bShouldDrawCells = false;
}

UObject* UMontageTrack_Sequences::GenerateNewDataAsset(UObject* Outer, FName Name)
{
	// UMGEdNode_Montage* MontageEdNode = Cast<UMGEdNode_Montage>(NodeBase);
	// UMGNode_Montage* MontageNode = Cast<UMGNode_Montage>(MontageEdNode->RuntimeNode);
	//
	// FName MontageName = FName(OwningGraph->GetName() + '_' + MontageEdNode->
	// 	                                                   MontageDisplayName);

	UAnimMontage* NewMontage = NewObject<UAnimMontage>(Outer, Name, RF_Public | RF_Standalone);


	if (Sections.IsEmpty())
	{
		return NewMontage;
	}

	if (UMontageTrackSection_Sequences* MontageSection = Cast<UMontageTrackSection_Sequences>(Sections[0]))
	{
		UAnimSequence* AnimAsset = MontageSection->AnimSequence;
		if (!AnimAsset)
		{
			return nullptr;
		}
		USkeleton* AssetSkeleton = AnimAsset->GetSkeleton();
		if (!AssetSkeleton)
		{
			return nullptr;
		}


		NewMontage->SetSkeleton(AssetSkeleton);
	}

	// Build track structure
	TMap<FName, int32> SlotToTrackIndex;
	TArray<float>      TrackLengths;

	// Collect unique slot names and create tracks
	TSet<FName> UniqueSlotNames;
	for (const auto Section : Sections)
	{
		if (UMontageTrackSection_Sequences* MontageSection = Cast<UMontageTrackSection_Sequences>(Section))
		{
			UniqueSlotNames.Add(MontageSection->SlotName);
		}
	}

	NewMontage->SlotAnimTracks.Reset();
	for (const FName& SlotName : UniqueSlotNames)
	{
		int32                TrackIndex = NewMontage->SlotAnimTracks.Num();
		FSlotAnimationTrack& NewTrack   = NewMontage->AddSlot(SlotName);
		NewTrack.SlotName               = SlotName;
		SlotToTrackIndex.Add(SlotName, TrackIndex);
		TrackLengths.Add(0.0f);
	}

	// Process segments and build animation data
	float              TotalMontageLength = 0.0f;
	TMap<FName, float> SlotCurrentTime;

	for (const auto Section : Sections)
	{
		if (UMontageTrackSection_Sequences* MontageSection = Cast<UMontageTrackSection_Sequences>(Section))
		{
			UAnimSequence* Anim          = MontageSection->AnimSequence;
			float          SegmentLength = Anim->GetPlayLength();


			// Create animation segment
			FAnimSegment NewSegment;
			NewSegment.SetAnimReference(Anim, true);
			NewSegment.LoopingCount = 1;
			NewSegment.StartPos     = SlotCurrentTime.FindOrAdd(MontageSection->SlotName, 0.0f);

			int32 TrackIndex = SlotToTrackIndex[MontageSection->SlotName];
			NewMontage->SlotAnimTracks[TrackIndex].AnimTrack.AnimSegments.Add(NewSegment);

			// Create composite section
			FCompositeSection NewSection;
			NewSection.SectionName = MontageSection->SectionName;
			NewSection.Link(Anim, SegmentLength);
			NewSection.SetTime(SlotCurrentTime[MontageSection->SlotName]);
			NewMontage->CompositeSections.Add(NewSection);

			// Update timing tracking
			SlotCurrentTime[MontageSection->SlotName] += SegmentLength;
			TrackLengths[TrackIndex] = FMath::Max(TrackLengths[TrackIndex], SlotCurrentTime[MontageSection->SlotName]);
		}
	}
	// Calculate total montage length
	for (float TrackLength : TrackLengths)
	{
		TotalMontageLength = FMath::Max(TotalMontageLength, TrackLength);
	}
	//    
	//    // Apply blend settings
	// NewMontage->BlendIn = FAlphaBlend(MontageEdNode->BlendInSettings.Blend);
	// NewMontage->BlendModeIn = MontageEdNode->BlendInSettings.BlendMode;
	// NewMontage->BlendProfileIn = MontageEdNode->BlendInSettings.BlendProfile;
	//
	// NewMontage->BlendOut = FAlphaBlend(MontageEdNode->BlendOutSettings.Blend);
	// NewMontage->BlendModeOut = MontageEdNode->BlendOutSettings.BlendMode;
	// NewMontage->BlendProfileOut = MontageEdNode->BlendOutSettings.BlendProfile;
	//
	// NewMontage->BlendOutTriggerTime = MontageEdNode->InBlendOutTriggerTime;
	//    
	// Finalize montage
	NewMontage->SetCompositeLength(TotalMontageLength);


	// Trigger post-edit to ensure proper setup
	FProperty*            ChangedProperty = nullptr;
	FPropertyChangedEvent PropertyEvent(ChangedProperty);
	NewMontage->PostEditChangeProperty(PropertyEvent);

	MG_LOG("Created montage %s with %d segments and length %.2f",
	       *NewMontage->GetName(), Sections.Num(), TotalMontageLength);
	return NewMontage;
}

void SMGSequenceTrack::MakeSectionContextMenu(FMenuBuilder& ContextMenuBuilder)
{
	static constexpr float  ThumbnailIconSize       = 64.0f;
	static constexpr uint32 ThumbnailIconResolution = 64;
	
	if (!(TrackModel) || !(TrackModel->ObjPtr))
	{
		return;
	}

	if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(TrackModel->ObjPtr))
	{
		if (!SequencesTrack->Sections.IsValidIndex(SelectedSectionIndex))
		{
			return;
		}
		UMontageTrackSection_Sequences* MontageSection =
			Cast<UMontageTrackSection_Sequences>(
				SequencesTrack->Sections[SelectedSectionIndex]);
		if (!MontageSection)
		{
			return;
		}

		UAnimSequence* AnimSequence = MontageSection->AnimSequence;
		if (!AnimSequence) { return; }

		ContextMenuBuilder.BeginSection(TEXT("MontageSection"), LOCTEXT("MontageSection", "Montage Section"));
		{
			FAssetThumbnailConfig ThumbnailConfig;
			//
			// if (FSlateApplication::Get().InKismetDebuggingMode())
			// {
			// 	ThumbnailConfig.bForceGenericThumbnail = true;
			// }


			TSharedPtr<FAssetThumbnail> Thumb = MakeShared<FAssetThumbnail>(AnimSequence,
			                                                                ThumbnailIconResolution,
			                                                                ThumbnailIconResolution,
			                                                                UThumbnailManager::Get().
			                                                                GetSharedThumbnailPool());
			auto AnimSectionDetails = SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("NoBorder"))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.AutoWidth()
					[
						SNew(SBox)
						.MaxDesiredHeight(ThumbnailIconSize)
						.MaxDesiredWidth(ThumbnailIconSize)
						[
							Thumb->MakeThumbnailWidget(ThumbnailConfig)
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Fill)
					.Padding(FMargin(5, 0))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(AnimSequence->GetName()))
						]
						+ SVerticalBox::Slot()
						.Padding(FMargin(0, 5))
						.AutoHeight()
						.HAlign(HAlign_Fill)
						[
							SNew(SAnimMontageSlotPicker, AnimSequence, MontageSection->SlotName)
							.OnSlotChanged(FOnSlotChanged::CreateLambda([this](const FName NewSlotName)
							{
								if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(
									TrackModel->ObjPtr))
								{
									if (UMontageTrackSection_Sequences* MontageSections =
										Cast<UMontageTrackSection_Sequences>(
											SequencesTrack->Sections[SelectedSectionIndex]))
									{
										MontageSections->SlotName = NewSlotName;
										TrackModel->ObjPtr->OnTrackPropertiesChanged.Broadcast();
									}
								}
							}))
						]
					]
				];


			ContextMenuBuilder.AddWidget(AnimSectionDetails, FText::GetEmpty(), true);
			FUIAction ItemAction(FExecuteAction::CreateLambda([this]()
			{
				TrackModel->DeleteSection(SelectedSectionIndex);
			}));
			ContextMenuBuilder.AddMenuEntry(FText::FromString("Delete Section"), TAttribute<FText>(),
			                                FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage"),
			                                ItemAction);
		}
		ContextMenuBuilder.EndSection();
	}
}

void UMontageTrack_Sequences::DroppedAssetsOnTrack(TArray<FAssetData> Array)
{
	bool bBroadcastChanges = false;
	for (auto Asset : Array)
	{
		if (Asset.IsInstanceOf(UAnimSequence::StaticClass()))
		{
			if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(Asset.GetAsset()))
			{
				UMontageTrackSection_Sequences* NewMontageSection = NewObject<UMontageTrackSection_Sequences>(this);
				NewMontageSection->AnimSequence = AnimSequence;

				Sections.Add(NewMontageSection);

				bBroadcastChanges = true;
			}
		}
	}
	if (bBroadcastChanges)
	{
		OnTrackPropertiesChanged.Broadcast();
	}
}

TSharedRef<SWidget> UMontageTrack_Sequences::MakeTrackTimelineWidget(
	TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
{
	return SNew(SMGSequenceTrack, TrackModelPtr);
}

const FText UMontageTrack_Sequences::GetTrackName() const
{
	return FText::FromString("Sequences");
}

const FLinearColor UMontageTrack_Sequences::GetTrackColor() const
{
	return FColor(80, 123, 72, 255);
}

FSlateIcon UMontageTrack_Sequences::GetTrackIcon() const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimSequence");
}
#undef LOCTEXT_NAMESPACE
#endif
