#include "MontageTrack_GameplayEffect.h"

#include "MontageGraphEditorStyle.h"
#include "MontageTrack_BlendLinks.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/Tracers/MontageCollisionTracer.h"


UMontageTrack_GameplayEffect::UMontageTrack_GameplayEffect(const FObjectInitializer& ObjectInitializer)
{
}

UObject* UMontageTrack_GameplayEffect::GenerateNewDataAsset(UObject* Outer, FName Name)
{
	UMontageGraph*    OwnerGraph      = Cast<UMontageGraph>(Outer->GetOuter());
	// UCollisionTracer* TracerData      = NewObject<UCollisionTracer>(OwnerGraph, Name, RF_Public | RF_Standalone);
	// UAnimMontage*     MontageToRender = Cast<UAnimMontage>(Outer);

	for (UDopeSheetTrackSection* Section : Sections)
	{
		auto EffectSection = Cast<UMontageTrackSection_GameplayEffect>(Section);
		
		// //Copy TargetEffects
		// for (auto Effect : EffectSection->AppliedEffects)
		// {
		// 	int EffectIndex = OwnerGraph->GameplayEffects.AddUnique(Effect.LoadSynchronous());
		// 	TracerData->AnimSampleRange.TargetGameplayEffectIndices.Add(EffectIndex);
		// }
	}

	return nullptr;
}

void UMontageTrack_GameplayEffect::KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame)
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "RemoveDopeSheetSection_Transaction",
	                                               "Add Timeline section"));

	Modify(true);

	auto NewSection       = NewObject<UMontageTrackSection_GameplayEffect>(this);
	NewSection->StartTime = SelectionStartFrame;
	NewSection->EndTime   = SelectionEndFrame;
	Sections.Add(NewSection);

	OnTrackPropertiesChanged.Broadcast();
}

const FText UMontageTrack_GameplayEffect::GetTrackName() const
{
	return FText::FromString("GameplayEffect");
}

const FLinearColor UMontageTrack_GameplayEffect::GetTrackColor() const
{
	return FColor(190, 82, 0, 255);
}

FSlateIcon UMontageTrack_GameplayEffect::GetTrackIcon() const
{
	return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Icon.Tracers");
}
