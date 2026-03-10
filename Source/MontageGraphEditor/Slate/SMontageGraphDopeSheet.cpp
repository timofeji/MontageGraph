// Copyright Drop Games Inc.


#include "SMontageGraphDopeSheet.h"

#include "SlateOptMacros.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/SAnimDopeSheet.h"
#include "DopeSheet/Tracks/DopeSheetTrackViewModel.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Graph/EdNodes/MGEdNode_Montage.h"
#include "Styling/AppStyle.h"
#include "Tracks/MontageTrack_Sequences.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"

#define LOCTEXT_NAMESPACE "SMontageGraphDopeSheet"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMontageGraphDopeSheet::Construct(const FArguments&                 InArgs,
                                          TSharedPtr<FDopeSheetController> TimelineController)
{
	Controller = TimelineController;

	ChildSlot
	[
		SNew(SBorder)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(0)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(DopeSheetWidget, SAnimDopeSheet, Controller)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(4.f, 4.f)
			[
				SNew(SComboButton)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.HasDownArrow(false)
				.ToolTipText(LOCTEXT("DopeSheetOptionsTooltip", "DopeSheet view options"))
				.OnGetMenuContent_Lambda([this]() -> TSharedRef<SWidget>
				{
					FMenuBuilder MenuBuilder(false, nullptr);
					MenuBuilder.AddMenuEntry(
						LOCTEXT("HorizontalLayoutLabel", "Side by Side"),
						LOCTEXT("HorizontalLayoutTooltip", "Show selected nodes side by side with blend-time overlap"),
						FSlateIcon(),
						FUIAction(
							FExecuteAction::CreateLambda([this]()
							{
								bHorizontalLayout = !bHorizontalLayout;
								RefreshLayout();
							}),
							FCanExecuteAction(),
							FIsActionChecked::CreateLambda([this]() { return bHorizontalLayout; })
						),
						NAME_None,
						EUserInterfaceActionType::ToggleButton
					);
					return MenuBuilder.MakeWidget();
				})
				.ButtonContent()
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Settings"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SMontageGraphDopeSheet::OnPaint(const FPaintArgs&        Args, const FGeometry& AllottedGeometry,
                                         const FSlateRect&        MyCullingRect,
                                         FSlateWindowElementList& OutDrawElements, int32 LayerId,
                                         const FWidgetStyle&      InWidgetStyle,
                                         bool                     bParentEnabled) const
{
	LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                         bParentEnabled);


	return LayerId;
}

void SMontageGraphDopeSheet::OnMontageSequencesUpdated()
{
	if (SelectedNodes.Num() == 0)
	{
		return;
	}

	for (UMGEdNode_Montage* Node : SelectedNodes)
	{
		Node->MarkStale();
		Node->MarkPackageDirty();
	}

	OnUpdateNodes.ExecuteIfBound();
	RefreshLayout();
}

void SMontageGraphDopeSheet::SetMontageCellViews()
{
	if (!Controller || SelectedNodes.Num() == 0)
	{
		return;
	}

	Controller->Reset();
	double AccumulatedPlayTime = 0.0;

	for (int32 NodeIdx = 0; NodeIdx < SelectedNodes.Num(); NodeIdx++)
	{
		UMGEdNode_Montage* Node = SelectedNodes[NodeIdx];
		if (Node->MontageTracks.Num() == 0)
		{
			continue;
		}

		if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(Node->MontageTracks[0]))
		{
			double NodeDuration  = 0.0;
			float  BlendOutTime  = 0.f;

			for (int i = 0; i < SequencesTrack->Sections.Num(); i++)
			{
				if (UMontageTrackSection_Sequences* MontagePart = Cast<UMontageTrackSection_Sequences>(
					SequencesTrack->Sections[i]))
				{
					if (!MontagePart->AnimSequence) { continue; }
					const double PlayLength = MontagePart->AnimSequence->GetPlayLength();

					Controller->AddViewSection(FDopeSheetViewSection(
						MontagePart->AnimSequence->GetNumberOfSampledKeys() - 1,
						AccumulatedPlayTime + NodeDuration,
						AccumulatedPlayTime + NodeDuration + PlayLength));

					NodeDuration += PlayLength;
					BlendOutTime  = MontagePart->BlendOutSettings.Blend.BlendTime;
				}
			}

			// In horizontal mode, next node starts (blendOut) seconds before this node ends
			const double Advance = (bHorizontalLayout && NodeIdx < SelectedNodes.Num() - 1)
				? FMath::Max(0.0, NodeDuration - static_cast<double>(BlendOutTime))
				: NodeDuration;

			AccumulatedPlayTime += Advance;
		}
	}
}


void SMontageGraphDopeSheet::SetSelection(UMGEdNode_Montage* NewSelection)
{
	TArray<TArray<UMGEdNode_Montage*>> SingleChain;
	SingleChain.Add({NewSelection});
	SetMultiSelection(SingleChain);
}

void SMontageGraphDopeSheet::SetMultiSelection(const TArray<TArray<UMGEdNode_Montage*>>& OrderedChains)
{
	// Unbind from all previous nodes
	for (UMGEdNode_Montage* Node : SelectedNodes)
	{
		if (Node && Node->MontageTracks.Num() > 0)
		{
			Node->MontageTracks[0]->OnTrackPropertiesChanged.RemoveAll(this);
		}
	}

	SelectedNodes.Empty();
	CombinedTracks.Empty();

	// Flatten all chains into SelectedNodes and build CombinedTracks
	for (const TArray<UMGEdNode_Montage*>& Chain : OrderedChains)
	{
		for (UMGEdNode_Montage* Node : Chain)
		{
			SelectedNodes.Add(Node);

			// Bind change delegate
			if (Node->MontageTracks.Num() > 0)
			{
				Node->MontageTracks[0]->OnTrackPropertiesChanged.AddRaw(
					this, &SMontageGraphDopeSheet::OnMontageSequencesUpdated);

				// Add tracks to combined list
				for (UDopeSheetTrackBase* Track : Node->MontageTracks)
				{
					CombinedTracks.Add(Track);
				}
			}
		}
	}

	if (CombinedTracks.Num() > 0)
	{
		SetMontageCellViews();
		Controller->SetTracksSource(CombinedTracks, SelectedNodes.Num() > 0 ? SelectedNodes[0] : nullptr);
		RefreshTrackOffsets();
		Controller->FrameViewRange();
	}
}

void SMontageGraphDopeSheet::RefreshLayout()
{
	if (!Controller || SelectedNodes.Num() == 0)
	{
		return;
	}

	SetMontageCellViews();
	RefreshTrackOffsets();
	Controller->FrameViewRange();
}

void SMontageGraphDopeSheet::RefreshTrackOffsets()
{
	if (!Controller)
	{
		return;
	}

	// Reset all offsets — vertical mode keeps every track at 0
	for (auto& TrackModel : Controller->RootTracks)
	{
		TrackModel->DisplayTimeOffset = 0.0;
	}

	if (!bHorizontalLayout || SelectedNodes.Num() <= 1)
	{
		return;
	}

	double AccumulatedOffset = 0.0;
	for (int32 NodeIdx = 0; NodeIdx < SelectedNodes.Num(); NodeIdx++)
	{
		UMGEdNode_Montage* Node = SelectedNodes[NodeIdx];

		// Apply offset to every track ViewModel belonging to this node
		for (UDopeSheetTrackBase* Track : Node->MontageTracks)
		{
			for (auto& TrackModel : Controller->RootTracks)
			{
				if (TrackModel->ObjPtr == Track)
				{
					TrackModel->DisplayTimeOffset = AccumulatedOffset;
					break;
				}
			}
		}

		// Compute offset for the next node: this node's duration minus its blend-out time
		if (NodeIdx < SelectedNodes.Num() - 1 && Node->MontageTracks.Num() > 0)
		{
			if (UMontageTrack_Sequences* SeqTrack = Cast<UMontageTrack_Sequences>(Node->MontageTracks[0]))
			{
				double NodeDuration = 0.0;
				float  BlendOutTime = 0.f;
				for (UDopeSheetTrackSection* Sec : SeqTrack->Sections)
				{
					if (UMontageTrackSection_Sequences* SeqSec = Cast<UMontageTrackSection_Sequences>(Sec))
					{
						if (SeqSec->AnimSequence)
						{
							NodeDuration += SeqSec->AnimSequence->GetPlayLength();
						}
						BlendOutTime = SeqSec->BlendOutSettings.Blend.BlendTime;
					}
				}
				AccumulatedOffset += FMath::Max(0.0, NodeDuration - static_cast<double>(BlendOutTime));
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
