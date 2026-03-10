//Created by Timofej Jermolaev, All rights reserved .

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MGNode.h"
#include "MGNode_Montage.generated.h"


class UGameplayEffect;
class UCollisionTracer;
class UAnimMontage;


/**
 * Per-node container for all data baked from editor tracks at compile time.
 *
 * Core gameplay assets (Montage, CollisionTracer) live as direct typed fields for zero-overhead
 * access on the hot path.  Future track types write into ExtendedPayloads keyed by FName so new
 * data can be added without touching this struct or RegenerateMontage.
 */
USTRUCT(Blueprintable)
struct MONTAGEGRAPH_API FMGBakedNodeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Baked")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(VisibleAnywhere, Category = "Baked")
	TObjectPtr<UCollisionTracer> CollisionTracer;

	/** Extension slot: future track types write named payloads here without modifying this struct. */
	UPROPERTY(VisibleAnywhere, Category = "Baked")
	TMap<FName, TObjectPtr<UObject>> ExtendedPayloads;

	template <typename T>
	T* GetExtended(FName Key) const
	{
		if (const TObjectPtr<UObject>* Found = ExtendedPayloads.Find(Key))
		{
			return Cast<T>(Found->Get());
		}
		return nullptr;
	}
};




/**
 * 
 */
USTRUCT(BlueprintType)
struct MONTAGEGRAPH_API FMontageGraphLinkSettings 
{
	GENERATED_BODY()

	

	UPROPERTY()
	double StartTime = 0.f;
	
	/*Skip ahead time, relative to Montage Beginning*/
	UPROPERTY(EditAnywhere, Category = "Settings")
	double TargetLinkToTime = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	FMontageBlendSettings BlendSettings;


	
	FMontageGraphLinkSettings(double InStartTime)
		: StartTime(InStartTime)
	{
		
	}
	
	FMontageGraphLinkSettings()
	{
		
	};
	
};

/**
 *  Base Class for MG Animation nodes acting based on an Anim Montage or Sequence asset.
 *
 *  Holds runtime properties for animation and effects / cues containers.
 */
UCLASS(Blueprintable)
class MONTAGEGRAPH_API UMGNode_Montage : public UMGNode
{
	GENERATED_BODY()

public:
	UMGNode_Montage();
	
	UPROPERTY()
	TMap<UMGNode*, FMontageGraphLinkSettings> BlendLinks;

	/** All data baked from editor tracks. Authoritative source; graph arrays are derived caches. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Baked")
	FMGBakedNodeData BakedData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
	FGameplayTagContainer AggregatedTags;
	

#if WITH_EDITOR
	virtual bool SupportsAssetClass(UClass* AssetClass);
	virtual FText GetNodeTitle() const override;
	virtual FText GetAnimAssetLabel() const;
	virtual FText GetAnimAssetLabelTooltip() const;
	virtual FLinearColor GetBackgroundColor() const override;
#endif
};
