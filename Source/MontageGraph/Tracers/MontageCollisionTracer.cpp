#include "MontageCollisionTracer.h"

#include "GameFramework/GameStateBase.h"
#include "MontageGraph/MontageGraphComponent.h"

DECLARE_STATS_GROUP(TEXT("MontageCollisionTracer"), STATGROUP_MontageCollision, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("MontageCollisionTracer Tick"), STAT_MontageCollisionTick, STATGROUP_MontageCollision);

int UCollisionTracer::GetCurrentFrame(float AnimAlpha) const
{
	const double CollisionAlphaInterval = AnimSampleRange.AnimEndAlpha - AnimSampleRange.AnimStartAlpha;
	const double EffectiveAlpha         = (AnimAlpha - AnimSampleRange.AnimStartAlpha) / CollisionAlphaInterval;

	const int MaxIndex = SamplePositions.Num() - 2;
	const int MinIndex = 0;

	int IndexAlpha = MaxIndex * EffectiveAlpha;
	return FMath::Clamp(IndexAlpha, MinIndex, MaxIndex);
}

FMontageCollisionTracerTickFunction::FMontageCollisionTracerTickFunction()
{
	bHighPriority = true;
	bRunOnAnyThread = false;
	bAllowTickBatching = false;
}

//
//
void FMontageCollisionTracerTickFunction::ExecuteTick(float                 DeltaTime, ELevelTick TickType,
                                                      ENamedThreads::Type   CurrentThread,
                                                      const FGraphEventRef& MyCompletionGraphEvent)
{
	SCOPE_CYCLE_COUNTER(STAT_MontageCollisionTick);

	if (!Target || !Target->IsValidLowLevel())
	{
		return;
	}

	if (DeltaTime <= SMALL_NUMBER)
	{
		return;
	}
	
	

	if (AGameStateBase* GameState = Target->GetWorld()->GetGameState())
	{
		UWorld* const World             = GameState->GetWorld();
		const float   WorldTimeDilation = World ? World->GetWorldSettings()->TimeDilation : 1.f;

		float CustomTimeDilation = 1.0f;
		if (AActor* TargetOwner = Target->GetOwner())
		{
			CustomTimeDilation = TargetOwner->CustomTimeDilation;
		}
		// Accumulate dilated time progress
		const float DilatedDeltaTime = DeltaTime *  WorldTimeDilation * CustomTimeDilation;
		TickInterval = DilatedDeltaTime;
       
		AccumulatedDilatedTime += DilatedDeltaTime;
       
		const float OriginalDuration = EndTime - StartTime;
		const float AnimAlpha = FMath::Clamp(AccumulatedDilatedTime / OriginalDuration, 0.f, 1.f);

		Target->ExecuteMontageCollisionTraceTick(AnimAlpha);


		if (AnimAlpha >= 1.f)
		{
			SetTickFunctionEnable(false);
		}
	}
}

FString FMontageCollisionTracerTickFunction::DiagnosticMessage()
{
	return FString::Printf(TEXT("MontageCollisionTracerTickFunction for %s"), 
            Target ? *Target->GetName() : TEXT("NULL"));
}

FName FMontageCollisionTracerTickFunction::DiagnosticContext(bool bDetailed)
{
	return FName(TEXT("MontageCollisionTracerTickFunction"));
}

