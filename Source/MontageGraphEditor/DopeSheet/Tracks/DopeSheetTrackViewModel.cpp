#include "DopeSheetTrackViewModel.h"

#include "DopeSheetTrackBase.h"
#include "IDopeSheetTrack.h"
#include "DopeSheet/DopeSheetController.h"

FDopeSheetTrackViewModel::FDopeSheetTrackViewModel(UDopeSheetTrackBase*             InObjPtr,
                                                   TSharedPtr<FDopeSheetController> InController)
	: ObjPtr(InObjPtr), Controller(InController)
{
	//Create View Models
	for (int SectionIndex = 0; SectionIndex < InObjPtr->Sections.Num(); SectionIndex++)
	{
		auto Section = InObjPtr->Sections[SectionIndex];
		if (Section == nullptr)
		{
			continue;
		}

		SectionModels.Add(FDopeSheetSectionViewModel(Section->StartTime, Section->EndTime));
	}
}


FDopeSheetTrackViewModel::~FDopeSheetTrackViewModel()
{
}

TSharedRef<SWidget> FDopeSheetTrackViewModel::MakeTrackTimelineWidget()
{
	if (!ObjPtr)
	{
		return SNullWidget::NullWidget;
	}


	return SNew(SBox)
		.MinDesiredHeight(32.f)
		.MaxDesiredHeight(32.f)
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
			.BorderBackgroundColor(ObjPtr->GetTrackColor().Desaturate(.75f))
			.Visibility_Lambda([this] { return IsVisible() ? EVisibility::Visible : EVisibility::Collapsed; })
			.Padding(0)
			[
				ObjPtr->MakeTrackTimelineWidget(SharedThis(this))
			]
		];
}

void FDopeSheetTrackViewModel::DeleteSection(int32 SectionIndex)
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "RemoveDopeSheetSection_Transaction",
	                                               "Remove Timeline section"));

	if (ObjPtr->Sections.IsValidIndex(SectionIndex))
	{
		if (UDopeSheetTrackSection* Section = ObjPtr->Sections[SectionIndex])
		{
			Section->MarkAsGarbage();
			ObjPtr->Modify(true);
			ObjPtr->Sections.RemoveAt(SectionIndex);
			ObjPtr->OnTrackPropertiesChanged.Broadcast();
		}
	}
}

void FDopeSheetTrackViewModel::ShiftSection(int32 SectionIndex, double MoveTimeDelta)
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "MoveDopeSheetSection_Transaction",
	                                               "Move Timeline section"));

	if (auto Section = ObjPtr->Sections[SectionIndex])
	{
		Section->Modify(true);
		Section->StartTime += MoveTimeDelta;
		Section->EndTime += MoveTimeDelta;

		SectionModels[SectionIndex].StartTime = Section->StartTime;
		SectionModels[SectionIndex].EndTime = Section->EndTime;
	}

	ObjPtr->OnTrackPropertiesChanged.Broadcast();
}

float FDopeSheetTrackViewModel::GetTrackHeight()
{
	return 32.f;
}

bool FDopeSheetTrackViewModel::IsVisible()
{
	return Controller->VisibleTracks.Contains(ObjPtr);
}

FLinearColor FDopeSheetTrackViewModel::GetTrackColor()
{
	if (ObjPtr)
	{
		return ObjPtr->GetTrackColor();
	}
	return FLinearColor::White;
}

void FDopeSheetTrackViewModel::DroppedAssetsOnTrack(TArray<FAssetData> Array)
{
}
