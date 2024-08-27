
#include "ActionGraphFactory.h"
#include "ActionGraph/ActionGraph.h"

#define LOCTEXT_NAMESPACE "UHBActionGraphFactory"

UActionGraphFactory::UActionGraphFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UActionGraph::StaticClass();

	// If we ever need to pick up a different Parent class in editor (have to implement ConfigureProperties())
	// ParentClass = UHBActionGraphAbility::StaticClass();
}

bool UActionGraphFactory::ConfigureProperties()
{
	// TSharedRef<SGameplayAbilityBlueprintCreateDialog> Dialog = SNew(SGameplayAbilityBlueprintCreateDialog);
	// return Dialog->ConfigureProperties(this);
	return Super::ConfigureProperties();
}

UObject* UActionGraphFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UActionGraph::StaticClass()));
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE
