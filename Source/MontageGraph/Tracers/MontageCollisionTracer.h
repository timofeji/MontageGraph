#pragma once
#include "GameplayPrediction.h"

#include "MontageCollisionTracer.generated.h"

struct FProcMeshTangent;
struct FCollisionShape;
class UMontageGraphComponent;

USTRUCT()
struct FCollisionEffectiveRange
{
	GENERATED_BODY()

	UPROPERTY()
	double AnimStartAlpha;
	
	UPROPERTY()
	double AnimEndAlpha;

	FCollisionEffectiveRange(float A, float B)
		: AnimStartAlpha(A),
		  AnimEndAlpha(B)
	{
	}


	FCollisionEffectiveRange()
		: AnimStartAlpha(0.f),
		  AnimEndAlpha(0.f)
	{
	}
};

UCLASS()
class MONTAGEGRAPH_API UCollisionTracer : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FCollisionEffectiveRange AnimSampleRange;

	UPROPERTY()
	FVector CollisionExtent;
	
	UPROPERTY()
	TArray<FVector> SamplePositions;
	
	UPROPERTY()
	TArray<FQuat> SampleOrientations;
	
	
	UPROPERTY()
	TArray<FVector> Vertices;

	UPROPERTY()
	TArray<FVector> Normals;

	UPROPERTY()
	TArray<FVector2D> UV0;
	
	UPROPERTY()
	TArray<FVector2D> UV1;

	UPROPERTY()
	TArray<int32> Indices;

	UPROPERTY()
	TArray<FLinearColor> VertexColors;
	
	UPROPERTY()
	TArray<FProcMeshTangent> Tangents;

	
	UPROPERTY()
	TEnumAsByte<ECollisionChannel> TraceChannel;


	int GetCurrentFrame(float AnimAlpha) const;
};

//
USTRUCT()
struct FMontageCollisionTracerTickFunction : public FTickFunction
{
	GENERATED_BODY()

	FMontageCollisionTracerTickFunction();
	
	UMontageGraphComponent* Target;

	float StartTime;
	float AccumulatedDilatedTime;
	float EndTime;

	// FTickFunction interface
	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
	virtual FString DiagnosticMessage() override;
	virtual FName DiagnosticContext(bool bDetailed) override;

	
};


template<>
struct TStructOpsTypeTraits<FMontageCollisionTracerTickFunction> : public TStructOpsTypeTraitsBase2<FMontageCollisionTracerTickFunction>
{
	enum
	{
		WithCopy = false
	};
};
