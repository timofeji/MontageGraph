// Copyright 2022 Mickael Daniel. All Rights Reserved.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "HBMontageGraphNodeBlueprints.generated.h"
//
// /**
//  * A HBMontageGraph NodeMontageBlueprint is essentially a simple UObject Blueprint with default node created when first opened.
//  *
//  * The HBMontageGraph factory should pick this for you automatically
//  */
// UCLASS(BlueprintType)
// class MONTAGEGRAPH_API UHBMontageGraphNodeMontageBlueprint : public UBlueprint
// {
// 	GENERATED_BODY()
//
// #if WITH_EDITOR
// 	// UBlueprint interface
// 	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
// 	// End of UBlueprint interface
// #endif
// };
//
// /**
//  * A HBMontageGraph NodeSequenceBlueprint is essentially a simple UObject Blueprint with default node created when first opened.
//  *
//  * The HBMontageGraph factory should pick this for you automatically
//  */
// UCLASS(BlueprintType)
// class MONTAGEGRAPH_API UHBMontageGraphNodeSequenceBlueprint : public UBlueprint
// {
// 	GENERATED_BODY()
//
// #if WITH_EDITOR
// 	// UBlueprint interface
// 	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
// 	// End of UBlueprint interface
// 	#endif
// };
