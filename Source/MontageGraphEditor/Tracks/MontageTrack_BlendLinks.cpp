// Copyright Drop Games Inc.


#include "MontageTrack_BlendLinks.h"

#include "MontageGraphEditorStyle.h"

UMontageTrack_BlendLinks::UMontageTrack_BlendLinks(const FObjectInitializer& ObjectInitializer)
{
	FDopeSheetKey DefaultBlendKey = FDopeSheetKey();
	Keys.Add(DefaultBlendKey);
}

const FText UMontageTrack_BlendLinks::GetTrackName() const
{
	return FText::FromString("Links");
}

const FLinearColor UMontageTrack_BlendLinks::GetTrackColor() const
{
	return FColor(49, 73, 44, 255);
}

FSlateIcon UMontageTrack_BlendLinks::GetTrackIcon() const
{
	return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Node.Icon.Selector");
}
