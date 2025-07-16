// Copyright Drop Games Inc. 

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MontageCollisionTrace.generated.h"

#if WITH_EDITOR
DECLARE_DELEGATE(FEditorTracerEvent)
#endif

DECLARE_MULTICAST_DELEGATE(FTracerEvent)

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class MONTAGEGRAPH_API UAnimNotifyState_MontageCollisionTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	/*GameplayEffects applied to tagets hit by this Collision Trace*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<TSubclassOf<UGameplayEffect>> TargetAppliedEffects;

	/*GameplayEffects applied to tagets hit by this Collision Trace*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FVector CollisionExtent = FVector(10.f, 10.f, 125.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FVector CollisionOffset = FVector(10.f, 10.f, 125.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FName SocketToSample = FName("weapon_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel2;
};
