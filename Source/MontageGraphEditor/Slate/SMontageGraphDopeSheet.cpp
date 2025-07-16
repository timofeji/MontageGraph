// Copyright Drop Games Inc.


#include "SMontageGraphDopeSheet.h"

#include "SlateOptMacros.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/SAnimDopeSheet.h"
#include "Graph/EdNodes/MGEdNode_Montage.h"
#include "Iris/ReplicationState/ReplicationStateUtil.h"
#include "Tracks/MontageTrack_Sequences.h"

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
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SAssignNew(DopeSheetWidget, SAnimDopeSheet, Controller)
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
	if (!SelectedNode)
	{
		return;
	}
	
	SelectedNode->MarkStale();
	OnUpdateNodes.ExecuteIfBound();
	
	SetMontageCellViews();

	SelectedNode->MarkPackageDirty();	
}

void SMontageGraphDopeSheet::SetMontageCellViews()
{
	if (!Controller || !SelectedNode)
	{
		return;
	}

	Controller->Reset();
	if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(
		SelectedNode->MontageTracks[0]))
	{
		double AccumulatedPlayTime = 0.f;
		for (int i = 0; i < SequencesTrack->Sections.Num(); i++)
		{
			if (UMontageTrackSection_Sequences* MontagePart = Cast<UMontageTrackSection_Sequences>(
				SequencesTrack->Sections[i]))
			{
				UAnimSequence* Sequence = MontagePart->AnimSequence;
				Controller->AddViewSection(FDopeSheetViewSection(Sequence->GetNumberOfSampledKeys(),
				                                                 AccumulatedPlayTime,
				                                                 AccumulatedPlayTime + Sequence->GetPlayLength()));
				AccumulatedPlayTime += Sequence->GetPlayLength();
			}
		}
	}
}


void SMontageGraphDopeSheet::SetSelection(UMGEdNode_Montage* NewSelection)
{
	if (SelectedNode)
	{
		if (NewSelection->MontageTracks.Num() > 0)
		{
			NewSelection->MontageTracks[0]->OnTrackPropertiesChanged.RemoveAll(this);
		}
	}


	if (NewSelection->MontageTracks.Num() > 0)
	{
		NewSelection->MontageTracks[0]->OnTrackPropertiesChanged.AddRaw(
			this, &SMontageGraphDopeSheet::OnMontageSequencesUpdated);
		
		SelectedNode = NewSelection;

		SetMontageCellViews();
		Controller->SetTracksSource(NewSelection->MontageTracks);
		Controller->FrameViewRange();
		
	}
}

