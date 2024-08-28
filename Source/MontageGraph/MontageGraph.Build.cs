// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MontageGraph : ModuleRules
{
	public MontageGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;				
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		PrivateDependencyModuleNames.AddRange(new string[] { "ControlRig" });
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"GameplayTags",
				"Core",
				"CoreUObject",
				// ... add other public dependencies that you statically link with here ...
			}
			);
		}
		
}
