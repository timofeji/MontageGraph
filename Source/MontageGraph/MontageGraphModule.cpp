// Copyright Timofej Jermolaev, All Rights Reserved.

#include "MontageGraphModule.h"
#include "MontageGraphGlobals.h"

#define LOCTEXT_NAMESPACE "FMontageGraphModule"

class UMontageGraphGlobals;

class FMontageGraphModule : public IMontageGraphModule
{
	// Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface


	virtual UMontageGraphGlobals* GetMontageGraphGlobals() override
	{
		// Defer loading of globals to the first time it is requested
		if (!MontageGraphGlobals)
		{
			// QUICK_SCOPE_CYCLE_COUNTER(STAT_IGameplayAbilitiesModule_GetAbilitySystemGlobals_LoadModule);
			// const UGameplayAbilitiesDeveloperSettings* Settings = GetDefault<UMontageGraphDeveloperSettings>();
			// FSoftClassPath AbilitySystemClassName = Settings->AbilitySystemGlobalsClassName;

			// UClass* SingletonClass = AbilitySystemClassName.TryLoadClass<UObject>();
			// checkf(SingletonClass != nullptr,
			//        TEXT("Ability config value AbilitySystemGlobalsClassName is not a valid class name."));

			MontageGraphGlobals = NewObject<UMontageGraphGlobals>(GetTransientPackage(),
			                                                      UMontageGraphGlobals::StaticClass(), NAME_None);
			MontageGraphGlobals->AddToRoot();
			MontageGraphGlobals->InitGlobalData();
			MontageGraphGlobalsReadyCallback.Broadcast();
		}

		check(MontageGraphGlobals);
		return MontageGraphGlobals;
	}

	virtual bool IsMontageGraphGlobalsAvailable() override
	{
		return MontageGraphGlobals != nullptr;
	}

	void CallOrRegister_OnMontageGraphGlobalsReady(FSimpleMulticastDelegate::FDelegate Delegate)
	{
		if (MontageGraphGlobals)
		{
			Delegate.Execute();
		}
		else
		{
			MontageGraphGlobalsReadyCallback.Add(Delegate);
		}
	}

	FSimpleMulticastDelegate MontageGraphGlobalsReadyCallback;
	UMontageGraphGlobals* MontageGraphGlobals;
};

void FMontageGraphModule::StartupModule()
{
	
	// This is loaded upon first request
	MontageGraphGlobals = nullptr;
//
// #if WITH_GAMEPLAY_DEBUGGER
// 	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
// 	GameplayDebuggerModule.RegisterCategory("MontageGraph", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Abilities::MakeInstance));
// 	GameplayDebuggerModule.NotifyCategoriesChanged();
// #endif // WITH_GAMEPLAY_DEBUGGER

}

void FMontageGraphModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMontageGraphModule, MontageGraph)

