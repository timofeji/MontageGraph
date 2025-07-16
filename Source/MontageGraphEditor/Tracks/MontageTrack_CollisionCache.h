#pragma once
#include "CoreMinimal.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "MontageTrack_CollisionCache.generated.h"


class UMontageGraph;
class UGameplayEffect;

#if WITH_EDITOR
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_CollisionCache : public UDopeSheetTrackSection
{
	GENERATED_BODY()

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	double Resolution = 180.f;


	/*CollisionEffects applied to tagets hit by this Collision Trace*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FVector CollisionExtent = FVector(10.f, 10.f, 125.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FVector CollisionOffset = FVector(0.f, 0.f, 125.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FName SocketToSample = FName("weapon_r");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FName SampleOrigin = FName("root");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel2;
	
	/*CollisionEffects applied to tagets hit by this Collision Trace*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<TSoftClassPtr<UGameplayEffect>> TargetAppliedEffects;
	
};


UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_CollisionCache : public UDopeSheetTrackBase 
{
public:
	GENERATED_BODY()
	UMontageTrack_CollisionCache(const FObjectInitializer& ObjectInitializer);

	virtual UObject* GenerateNewDataAsset(UObject* Outer, FName Name) override;

	virtual void KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame) override;

	/*Begin IDopeSheetTrack Interface*/
	virtual const FText        GetTrackName() const override;
	virtual const FLinearColor GetTrackColor() const override;
	virtual FSlateIcon   GetTrackIcon() const override;
	/*End IDopeSheetTrack Interface*/
	
};

#endif