// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class ActionGraphEditor: ModuleRules
{
	public ActionGraphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;				
		
		PublicIncludePaths.AddRange(
			new string[]
			{
				"ActionGraphEditor/",
			}
		);	
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"ActionGraphEditor/",
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
			"ActionGraph"
        			
		});
			
	      
	}
		
}
