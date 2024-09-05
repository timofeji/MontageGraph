// Created by Timofej Jermolaev, All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "MontageGraphFactory.generated.h"

UCLASS(HideCategories=Object, MinimalAPI)
class UMontageGraphFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMontageGraphFactory();

	// The type of blueprint that will be created
	UPROPERTY(EditAnywhere, Category=GameplayAbilitiesFactory)
	TEnumAsByte<enum EBlueprintType> BlueprintType;

	// The parent class of the created blueprint
	// UPROPERTY(EditAnywhere, Category=GameplayAbilitiesFactory)
	// TSubclassOf<class UGameplayAbility> ParentClass;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	//~ Begin UFactory Interface
};
