// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class MontageGraphEditor: ModuleRules
{
	public MontageGraphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;				
		
		PublicIncludePaths.AddRange(
			new string[]
			{
				"MontageGraphEditor/",
			}
		);	
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"MontageGraphEditor/",
			}
		);
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EditorStyle",
			"GraphEditor",
			"ToolWidgets",
			"GameplayTags",
			"UnrealEd",
		});
        
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Persona",
			"Core",
			"AssetTools",
			"ToolMenus",
			"CoreUObject",
			"Engine",
			"InputCore",
			"SlateCore",
			"Slate",
			"Niagara",
			"KismetCompiler",
			"Blutility",
			"AnimGraph",
			"BlueprintGraph",
			"KismetWidgets",
			"EditorScriptingUtilities",
			"AdvancedPreviewScene",
			"AnimationBlueprintLibrary",
			"MontageGraph"
        			
		});
			
	      
	}
		
}