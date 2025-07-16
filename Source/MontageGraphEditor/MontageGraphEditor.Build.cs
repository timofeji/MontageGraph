// Created by Timofej Jermolaev, All rights reserved. 
using UnrealBuildTool;

public class MontageGraphEditor: ModuleRules
{
	public MontageGraphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;				
		
		if (Target.bBuildEditor == false)
		{
			throw new BuildException("MontageGraphEditor is an editor-only module!");
		}
		
		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);	
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"MontageGraphEditor/",
			}
		);
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"Slate",
			"SlateCore",
			"ApplicationCore",
			"CoreUObject", "Engine", "InputCore",
			"Projects",
			"EditorStyle",
			"GraphEditor",
			"ToolWidgets",
			"GameplayTags",
			"GameplayAbilities",
			"UnrealEd", 
			"MovieSceneTools",
			
		});
        
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AnimationCore",
			"SequencerScriptingEditor",
			"Sequencer",
			"LevelSequence",
			"LevelSequenceEditor", 
			"MovieScene",
			"Persona",
			"AssetTools",
			"ToolMenus",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Niagara",
			"KismetCompiler",
			"Blutility",
			"AnimGraph",
			"BlueprintGraph",
			"KismetWidgets",
			"EditorScriptingUtilities",
			"AdvancedPreviewScene",
			"AnimationBlueprintLibrary",
			"EditorInteractiveToolsFramework",
			"MontageGraph"
		});
			
	      
	}
		
}