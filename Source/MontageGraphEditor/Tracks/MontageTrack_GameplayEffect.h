#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "MontageTrack_GameplayEffect.generated.h"


class UMontageGraph;
class UGameplayEffect;

#if WITH_EDITOR
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_GameplayEffect : public UDopeSheetTrackSection
{
	GENERATED_BODY()

	
public:
	/*Tags applied to the MontageGraphComponent owner when this section is active*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayTagContainer AppliedGameplayTags;
	
	/*Effects applied to the MontageGraphComponent owner when this section is active*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<TSoftClassPtr<UGameplayEffect>> AppliedEffects;
};


UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_GameplayEffect : public UDopeSheetTrackBase 
{
public:
	GENERATED_BODY()
	UMontageTrack_GameplayEffect(const FObjectInitializer& ObjectInitializer);

	virtual UObject* GenerateNewDataAsset(UObject* Outer, FName Name) override;

	virtual void KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame) override;

	/*Begin IDopeSheetTrack Interface*/
	virtual const FText        GetTrackName() const override;
	virtual const FLinearColor GetTrackColor() const override;
	virtual FSlateIcon   GetTrackIcon() const override;
	/*End IDopeSheetTrack Interface*/
	
};

#endif