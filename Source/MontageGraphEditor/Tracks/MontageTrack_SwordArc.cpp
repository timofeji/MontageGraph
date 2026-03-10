// Created by Timofej Jermolaev, All rights reserved.
#include "MontageTrack_SwordArc.h"

#include "AnimPose.h"
#include "MontageGraphEditorStyle.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/Tracers/SwordArcData.h"

const FName UMontageTrack_SwordArc::PayloadKey("SwordArc");

UMontageTrack_SwordArc::UMontageTrack_SwordArc(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAllowRename = false;
}

// ---------------------------------------------------------------------------
//  BakeToNode — allocate the runtime asset and insert it into ExtendedPayloads
// ---------------------------------------------------------------------------

void UMontageTrack_SwordArc::BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
                                         UMontageGraph* OwnerGraph, const FString& DisplayName)
{
	// Parent to the montage so the arc data travels with it; fall back to the graph.
	UObject* DataOuter = BakedData.Montage ? (UObject*)BakedData.Montage.Get() : (UObject*)OwnerGraph;
	USwordArcData* ArcData = CreateNewDataObject<USwordArcData>(DataOuter, FName(DisplayName + "_SwordArc"));
	BakedData.ExtendedPayloads.Add(PayloadKey, ArcData);
}

// ---------------------------------------------------------------------------
//  GenerateNewDataAsset — per-frame bake of tip velocity in root-local space
// ---------------------------------------------------------------------------

UObject* UMontageTrack_SwordArc::GenerateNewDataAsset(UObject* Outer, FName Name)
{
	UMontageGraph* OwnerGraph      = Cast<UMontageGraph>(Outer->GetOuter());
	USwordArcData* ArcData         = NewObject<USwordArcData>(OwnerGraph, Name, RF_Public | RF_Standalone);
	UAnimMontage*  MontageToRender = Cast<UAnimMontage>(Outer);

	if (!MontageToRender)
	{
		return ArcData;
	}

	TArray<UAnimationAsset*> MontageSequences;
	MontageToRender->GetAllAnimationSequencesReferred(MontageSequences);
	const double MontageLength = MontageToRender->GetPlayLength();

	if (MontageLength <= 0.0)
	{
		return ArcData;
	}

	for (UDopeSheetTrackSection* Section : Sections)
	{
		UMontageTrackSection_SwordArc* ArcSection = Cast<UMontageTrackSection_SwordArc>(Section);
		if (!ArcSection) { continue; }

		const double StartTime = ArcSection->StartTime;
		const double EndTime   = ArcSection->EndTime;

		ArcData->RangeStart = (float)(StartTime / MontageLength);
		ArcData->RangeEnd   = (float)(EndTime   / MontageLength);

		// --- Collect animation sequences that fall within this section's time range ---
		// (mirrors the CollisionCache pattern exactly)
		float AccumulatedTime = 0.f;
		TArray<UAnimSequence*> SequencesWithinSection;
		for (UAnimationAsset* Asset : MontageSequences)
		{
			if (UAnimSequence* AnimSeq = Cast<UAnimSequence>(Asset))
			{
				const float AnimLength = AnimSeq->GetPlayLength();
				if (StartTime > AccumulatedTime && EndTime < AccumulatedTime + AnimLength)
				{
					SequencesWithinSection.Add(AnimSeq);
					AccumulatedTime += AnimLength;
				}
			}
		}

		// --- Sample and bake ---
		for (UAnimSequence* AnimSeq : SequencesWithinSection)
		{
			const double StepTime = (1.0 / ArcSection->SampleMultiplier)
			                        * AnimSeq->GetDataModel()->GetFrameRate().AsInterval();

			TArray<double> SampleTimes;
			for (double T = StartTime; T <= EndTime; T += StepTime)
			{
				SampleTimes.Add(T);
			}

			if (SampleTimes.Num() < 2)
			{
				continue;
			}

			TArray<FAnimPose>          AnimPoses;
			FAnimPoseEvaluationOptions EvalOptions;
			UAnimPoseExtensions::GetAnimPoseAtTimeIntervals(AnimSeq, SampleTimes, EvalOptions, AnimPoses);

			// For each adjacent pose pair, compute the tip's velocity in root-local space.
			// Root-local tip position = WorldTip - WorldRoot, which cancels global translation.
			for (int32 i = 0; i < AnimPoses.Num() - 1; ++i)
			{
				const FVector CurRoot  = UAnimPoseExtensions::GetBonePose(AnimPoses[i],     ArcSection->RootSocket, EAnimPoseSpaces::World).GetLocation();
				const FVector CurTip   = UAnimPoseExtensions::GetBonePose(AnimPoses[i],     ArcSection->TipSocket,  EAnimPoseSpaces::World).GetLocation();
				const FVector NextRoot = UAnimPoseExtensions::GetBonePose(AnimPoses[i + 1], ArcSection->RootSocket, EAnimPoseSpaces::World).GetLocation();
				const FVector NextTip  = UAnimPoseExtensions::GetBonePose(AnimPoses[i + 1], ArcSection->TipSocket,  EAnimPoseSpaces::World).GetLocation();

				// Tip position in root-local space (strips world translation + root motion drift).
				const FVector CurTipLocal  = CurTip  - CurRoot;
				const FVector NextTipLocal = NextTip  - NextRoot;

				// Velocity direction — the direction the tip is sweeping between these two frames.
				const FVector RawDelta = NextTipLocal - CurTipLocal;
				const FVector SwingDir = RawDelta.GetSafeNormal();

				// Arc angle: signed angle of the swing direction projected into the root-local
				// horizontal plane.  X = forward, Y = right in standard UE skeleton conventions.
				//   0°   → blade tip moving directly forward
				//  +90°  → blade tip moving right
				//  -90°  → blade tip moving left
				const float ArcAngle = FMath::RadiansToDegrees(FMath::Atan2(SwingDir.Y, SwingDir.X));

				// Assign to the midpoint time between the two sampled frames.
				const double MidTime  = (SampleTimes[i] + SampleTimes[i + 1]) * 0.5;
				const float  MidAlpha = (float)(MidTime / MontageLength);

				ArcData->SampleAlphas.Add(MidAlpha);
				ArcData->SwingDirections.Add(SwingDir);
				ArcData->ArcAngles.Add(ArcAngle);
			}
		}
	}

	return ArcData;
}

// ---------------------------------------------------------------------------
//  Dope-sheet interaction helpers
// ---------------------------------------------------------------------------

void UMontageTrack_SwordArc::KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame)
{
	const FScopedTransaction Transaction(
		NSLOCTEXT("DopeSheet", "AddSwordArcSection_Transaction", "Add Sword Arc Section"));

	Modify(true);

	UMontageTrackSection_SwordArc* NewSection = NewObject<UMontageTrackSection_SwordArc>(this);
	NewSection->StartTime = SelectionStartFrame;
	NewSection->EndTime   = SelectionEndFrame;
	Sections.Add(NewSection);

	OnTrackPropertiesChanged.Broadcast();
}

const FText UMontageTrack_SwordArc::GetTrackName() const
{
	return FText::FromString("Sword Arc");
}

const FLinearColor UMontageTrack_SwordArc::GetTrackColor() const
{
	// Teal — visually distinct from the red Collision track and the grey Sequences track.
	return FLinearColor(0.f, 0.55f, 0.65f, 1.f);
}

FSlateIcon UMontageTrack_SwordArc::GetTrackIcon() const
{
	// Reuse the existing tracer icon; can be swapped for a dedicated one later.
	return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Icon.Tracers");
}
