#include "MontageTrack_CollisionCache.h"

#include "AnimPose.h"
#include "GameplayEffect.h"
#include "MontageGraphEditorStyle.h"
#include "ProceduralMeshComponent.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/Tracers/MontageCollisionTracer.h"

UMontageTrack_CollisionCache::UMontageTrack_CollisionCache(const FObjectInitializer& ObjectInitializer)
{
	bAllowRename = false;
}

void UMontageTrack_CollisionCache::BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
                                               UMontageGraph* OwnerGraph, const FString& DisplayName)
{
	// Collision data is parented to the montage so it travels with it; fall back to the graph
	// if the sequence track hasn't run yet (shouldn't happen in normal track order).
	UObject* MontageOuter = BakedData.Montage ? (UObject*)BakedData.Montage.Get() : (UObject*)OwnerGraph;
	BakedData.CollisionTracer = CreateNewDataObject<UCollisionTracer>(MontageOuter, FName(DisplayName + "_Collision"));
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


		TracerData->TraceChannel    = CollisionSection->TraceChannel;
		TracerData->CollisionExtent = CollisionSection->CollisionExtent;
		TracerData->AnimSampleRange = FCollisionEffectiveRange(startTime / MontageLength, endTime / MontageLength);

		
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

			const double StepTime   = (1/CollisionSection->SampleMultiplier) * AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

			
			TArray<double> SampleTimes;
			for (double i = startTime; i <= endTime; i+=StepTime )
			{
				SampleTimes.Add(i);
			}
			TArray<FAnimPose>          AnimPoses;
			FAnimPoseEvaluationOptions PoseEvaluationOptions;
			UAnimPoseExtensions::GetAnimPoseAtTimeIntervals(AnimSequence, SampleTimes,
				PoseEvaluationOptions,
				AnimPoses);

			/*Step through sequence at collision sweep sample times 
			 * to cache collision and preview tracer meshes*/
			double         ArcLenTop = 0.f;
			double         ArcLenBot = 0.f;
			for (int i = 0; i < AnimPoses.Num() - 1; i++)
			{

				const FTransform OriginSocketTransform = UAnimPoseExtensions::GetBonePose(
					AnimPoses[i], CollisionSection->SampleOrigin, EAnimPoseSpaces::World);
				const FTransform SocketTransform = UAnimPoseExtensions::GetBonePose(
					AnimPoses[i], CollisionSection->SocketToSample, EAnimPoseSpaces::World);
				const FTransform NextOriginSocketTransform = UAnimPoseExtensions::GetBonePose(
					AnimPoses[i + 1], CollisionSection->SampleOrigin, EAnimPoseSpaces::World);
				const FTransform NextSocketTransform = UAnimPoseExtensions::GetBonePose(
					AnimPoses[i + 1], CollisionSection->SocketToSample, EAnimPoseSpaces::World);


				const FTransform WeaponStart     = SocketTransform;
				const FTransform WeaponEnd       = FTransform(CollisionSection->CollisionOffset) * SocketTransform;
				const FTransform NextWeaponStart = NextSocketTransform;
				const FTransform NextWeaponEnd   = FTransform(CollisionSection->CollisionOffset) * NextSocketTransform;
/*
			Current Frame  →  Next Frame
                  ^
				 /*\ 3          2
				 ||| ●━━━━━━━━━━● 
				 ||| ////////////
				 ||| ////////////
				 ||| ///////////
SocketToSample > ||| ●━━━━━━━━━━●
  (weapon_r)	 ||| 0          1
			   ^\_*_/^
				  ║
				  ║  (hilt/handle)
			      ⇓
*/

				const FVector Origin     = OriginSocketTransform.GetLocation();
				const FVector NextOrigin = NextOriginSocketTransform.GetLocation();
				const FVector Loc0       = WeaponStart.GetLocation() - Origin;
				const FVector Loc3       = WeaponEnd.GetLocation() - Origin;
				const FVector Loc1       = NextWeaponStart.GetLocation() - NextOrigin;
				const FVector Loc2       = NextWeaponEnd.GetLocation() - NextOrigin;


				TracerData->SamplePositions.Add(Loc0 + .5f*(Loc3 - Loc0));
				TracerData->SampleOrientations.Add(SocketTransform.GetRotation());


				ArcLenTop += (Loc3 - Loc2).Length();
				ArcLenBot += (Loc0 - Loc1).Length();


				TracerData->Vertices.Add(Loc0);
				TracerData->Vertices.Add(Loc1);
				TracerData->Vertices.Add(Loc2);
				TracerData->Vertices.Add(Loc3);

				const int IndexOffset = (i) * 4;
				TracerData->Indices.Add(IndexOffset + 0);
				TracerData->Indices.Add(IndexOffset + 3);
				TracerData->Indices.Add(IndexOffset + 1);
				TracerData->Indices.Add(IndexOffset + 3);
				TracerData->Indices.Add(IndexOffset + 2);
				TracerData->Indices.Add(IndexOffset + 1);

				// float WidthBot = FVector::Distance(Loc1, Loc0);
				// float WidthTop = FVector::Distance(Loc2, Loc3);

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
				const FVector Loc0 = TracerData->Vertices[i];
				const FVector Loc1 = TracerData->Vertices[i + 1];
				const FVector Loc2 = TracerData->Vertices[i + 2];
				const FVector Loc3 = TracerData->Vertices[i + 3];


				double TopDist = (Loc3 - Loc2).Length();
				double BotDist = (Loc0 - Loc1).Length();

				double TopWidth = TopDist / ArcLenTop;
				double BotWidth = BotDist / ArcLenBot;

				TracerData->UV0.Add(FVector2D(BotLast, 1.f));
				TracerData->UV0.Add(FVector2D(BotLast + BotWidth, 1.f));
				TracerData->UV0.Add(FVector2D(TopLast + TopWidth, 0.f));
				TracerData->UV0.Add(FVector2D(TopLast, .0f));

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
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "RemoveDopeSheetSection_Transaction",
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
