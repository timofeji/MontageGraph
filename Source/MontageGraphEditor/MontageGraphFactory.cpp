
#include "MontageGraphFactory.h"
#include "MontageGraph/MontageGraph.h"

#define LOCTEXT_NAMESPACE "UHBMontageGraphFactory"

UMontageGraphFactory::UMontageGraphFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMontageGraph::StaticClass();

	// If we ever need to pick up a different Parent class in editor (have to implement ConfigureProperties())
	// ParentClass = UHBMontageGraphAbility::StaticClass();
}

bool UMontageGraphFactory::ConfigureProperties()
{
	// TSharedRef<SGameplayAbilityBlueprintCreateDialog> Dialog = SNew(SGameplayAbilityBlueprintCreateDialog);
	// return Dialog->ConfigureProperties(this);
	return Super::ConfigureProperties();
}

UObject* UMontageGraphFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UMontageGraph::StaticClass()));
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE
