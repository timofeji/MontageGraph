// Created by Timofej Jermolaev, All rights reserved. 

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
			}
		);
			
		PrivateDependencyModuleNames.AddRange(new string[] { "NetCore", "GameplayAbilities","GameplayTasks", "GameplayDebugger" });
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"GameplayTags",
				"Core",
				"CoreUObject",
				"ProceduralMeshComponent"
				// ... add other public dependencies that you statically link with here ...
			}
		);
	}
		
}
