#pragma once
#include "GameplayEffectTypes.h"
#include "MontageGraphGameplayEffectContext.generated.h"

class AActor;
class FArchive;
class UObject;

USTRUCT()
struct FMontageGraphGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FMontageGraphGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FMontageGraphGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	/** Returns the wrapped FEBGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static MONTAGEGRAPH_API FMontageGraphGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);
	

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FMontageGraphGameplayEffectContext* NewContext = new FMontageGraphGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FMontageGraphGameplayEffectContext::StaticStruct();
	}
	
	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
};




template<>
struct TStructOpsTypeTraits<FMontageGraphGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMontageGraphGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};


