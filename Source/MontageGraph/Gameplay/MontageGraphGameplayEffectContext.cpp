#include "MontageGraphGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MontageGraphGameplayEffectContext)

class FArchive;



FMontageGraphGameplayEffectContext* FMontageGraphGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FMontageGraphGameplayEffectContext::StaticStruct()))
	{
		return (FMontageGraphGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}


bool FMontageGraphGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	return true;
}
