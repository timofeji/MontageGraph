#include "MontageTrack_CollisionCache.h"

#include "AnimPose.h"
#include "GameplayEffect.h"
#include "MontageGraphEditorStyle.h"
#include "MontageTrack_BlendLinks.h"
#include "ProceduralMeshComponent.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/Tracers/MontageCollisionTracer.h"


UMontageTrack_CollisionCache::UMontageTrack_CollisionCache(const FObjectInitializer& ObjectInitializer)
{
}

UObject* UMontageTrack_CollisionCache::GenerateNewDataAsset(UObject* Outer, FName Name)
{
	UMontageGraph*    OwnerGraph      = Cast<UMontageGraph>(Outer->GetOuter());
	UCollisionTracer* TracerData      = NewObject<UCollisionTracer>(OwnerGraph, Name, RF_Public | RF_Standalone);
	UAnimMontage*     MontageToRender = Cast<UAnimMontage>(Outer);

	if (!MontageToRender)
	{
		return TracerData;
	}

	

	TArray<UAnimationAsset*> MontageSequences;
	MontageToRender->GetAllAnimationSequencesReferred(MontageSequences);
	const double MontageLength = MontageToRender->GetPlayLength();

	for (UDopeSheetTrackSection* Section : Sections)
	{
		auto CollisionSection = Cast<UMontageTrackSection_CollisionCache>(Section);
		if (!CollisionSection) { return TracerData; }

		const double startTime = CollisionSection->StartTime;
		const double endTime   = CollisionSection->EndTime;

		TArray<double> SampleTimes;
		float          ArcLenTop = 0.f;
		float          ArcLenBot = 0.f;

		double SweepInteral = endTime - startTime;
		for (double i = startTime; i < endTime; i += SweepInteral / CollisionSection->Resolution)
		{
			SampleTimes.Add(i);
		}

		TracerData->TraceChannel    = CollisionSection->TraceChannel;
		TracerData->CollisionExtent = CollisionSection->CollisionExtent;
		TracerData->AnimSampleRange = FCollisionEffectiveRange(startTime / MontageLength, endTime / MontageLength);

		//Copy TargetEffects
		for (auto Effect : CollisionSection->TargetAppliedEffects)
		{
			int EffectIndex = OwnerGraph->GameplayEffects.AddUnique(Effect.LoadSynchronous());
			TracerData->AnimSampleRange.TargetGameplayEffectIndices.Add(EffectIndex);
		}
        

		float                  AccumulatedTime = 0;
		TArray<UAnimSequence*> SequencesWithinSection;
		for (auto Sequence : MontageSequences)
		{
			if (auto AnimSequence = Cast<UAnimSequence>(Sequence))
			{
				const float AnimLength = AnimSequence->GetPlayLength();
				if (startTime > AccumulatedTime && endTime < AccumulatedTime + AnimLength)
				{
					SequencesWithinSection.Add(AnimSequence);
					AccumulatedTime += AnimLength;
				}
			}
		}


		for (UAnimSequence* AnimSequence : SequencesWithinSection)
		{
			TArray<FAnimPose>          AnimPoses;
			FAnimPoseEvaluationOptions PoseEvaluationOptions;
			UAnimPoseExtensions::GetAnimPoseAtTimeIntervals(AnimSequence, SampleTimes,
			                                                PoseEvaluationOptions,
			                                                AnimPoses);
			//Step through sequence at sweep notify times
			for (int i = 0; i < AnimPoses.Num() - 1; i++)
			{

				FTransform OriginSocketTransform = UAnimPoseExtensions::GetBonePose(
					AnimPoses[i], CollisionSection->SampleOrigin, EAnimPoseSpaces::World);
				
				FTransform NextSocketTransform = UAnimPoseExtensions::GetBonePose(
					AnimPoses[i], CollisionSection->SocketToSample, EAnimPoseSpaces::World);

				FTransform SocketTransform = UAnimPoseExtensions::GetBonePose(
					AnimPoses[i + 1], CollisionSection->SocketToSample, EAnimPoseSpaces::World);


				FTransform WeaponStart     = SocketTransform;
				FTransform WeaponEnd       = FTransform(CollisionSection->CollisionOffset) + SocketTransform;
				FTransform LastWeaponStart = NextSocketTransform;
				FTransform LastWeaponEnd   = FTransform(CollisionSection->CollisionOffset) + NextSocketTransform;

				//Cur      next
				/*__________
					  *|3///////2|- 
					  *|/////////|
					  *|/////////|
					  *|0///////1|- 
					   _|_
						|
					 */
				FVector Origin = OriginSocketTransform.GetLocation();
				FVector Loc0   = WeaponStart.GetLocation() - Origin;
				FVector Loc3   = WeaponEnd.GetLocation() - Origin;
				FVector Loc1   = LastWeaponStart.GetLocation() - Origin;
				FVector Loc2   = LastWeaponEnd.GetLocation() - Origin;


				TracerData->SamplePositions.Add(Loc0);
				TracerData->SampleOrientations.Add(SocketTransform.GetRotation());


				ArcLenTop += (Loc2 - Loc3).Length();
				ArcLenBot += (Loc1 - Loc0).Length();


				TracerData->Vertices.Add(Loc0);
				TracerData->Vertices.Add(Loc1);
				TracerData->Vertices.Add(Loc2);
				TracerData->Vertices.Add(Loc3);

				int IndexOffset = (i) * 4;
				TracerData->Indices.Add(IndexOffset + 0);
				TracerData->Indices.Add(IndexOffset + 3);
				TracerData->Indices.Add(IndexOffset + 1);
				TracerData->Indices.Add(IndexOffset + 3);
				TracerData->Indices.Add(IndexOffset + 2);
				TracerData->Indices.Add(IndexOffset + 1);

				float WidthBot = FVector::Distance(Loc1, Loc0);
				float WidthTop = FVector::Distance(Loc2, Loc3);

				FVector NormalVector = FVector::CrossProduct(Loc3 - Loc0, Loc1 - Loc0).GetSafeNormal();
				TracerData->Normals.Add(NormalVector);
				TracerData->Normals.Add(NormalVector);
				TracerData->Normals.Add(NormalVector);
				TracerData->Normals.Add(NormalVector);
			}

			double TopLast = 0.f;
			double BotLast = 0.f;
			for (int i = 0; i < TracerData->Vertices.Num(); i += 4)
			{
				FVector Loc0 = TracerData->Vertices[i];
				FVector Loc1 = TracerData->Vertices[i + 1];
				FVector Loc2 = TracerData->Vertices[i + 2];
				FVector Loc3 = TracerData->Vertices[i + 3];


				double TopDist = (Loc3 - Loc2).Length();
				double BotDist = (Loc0 - Loc1).Length();

				double TopWidth = TopDist / ArcLenTop;
				double BotWidth = BotDist / ArcLenBot;


				TracerData->UV0.Add(FVector2D(BotLast + BotWidth, 0.99f));
				TracerData->UV0.Add(FVector2D(BotLast, 0.99f));
				TracerData->UV0.Add(FVector2D(TopLast, 0.f));
				TracerData->UV0.Add(FVector2D(TopLast + TopWidth, .0f));

				TopLast += TopWidth;
				BotLast += BotWidth;
			}
		}
		TArray<FProcMeshTangent> Tangents;
		for (int i = 0; i < TracerData->Vertices.Num(); i += 4)
		{
			FVector Tangent = (TracerData->Vertices[i + 1] - TracerData->Vertices[i]).GetSafeNormal();
			Tangents.Add(FProcMeshTangent(Tangent, false));
			Tangents.Add(FProcMeshTangent(Tangent, false));
			Tangents.Add(FProcMeshTangent(Tangent, false));
			Tangents.Add(FProcMeshTangent(Tangent, false));
		}
		TracerData->Tangents.Append(Tangents);
	}

	return TracerData;
}

void UMontageTrack_CollisionCache::KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame)
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "RemoveDopeSheetAddSection_Transaction",
	                                               "Add Timeline section"));

	Modify(true);

	auto NewSection       = NewObject<UMontageTrackSection_CollisionCache>(this);
	NewSection->StartTime = SelectionStartFrame;
	NewSection->EndTime   = SelectionEndFrame;
	Sections.Add(NewSection);

	OnTrackPropertiesChanged.Broadcast();
}

const FText UMontageTrack_CollisionCache::GetTrackName() const
{
	return FText::FromString("Collision");
}

const FLinearColor UMontageTrack_CollisionCache::GetTrackColor() const
{
	return FColor(145, 5, 0, 255);
}

FSlateIcon UMontageTrack_CollisionCache::GetTrackIcon() const
{
	return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Icon.Tracers");
}
