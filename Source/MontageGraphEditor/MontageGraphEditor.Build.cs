// Created by Timofej Jermolaev, All rights reserved. 
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
			"Projects",
			"EditorStyle",
			"GraphEditor",
			"ToolWidgets",
			"GameplayTags",
			"UnrealEd", 
			"MovieSceneTools",
			"PropertyEditor"
		});
        
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AnimationCore",
			"ControlRig",
			"SequencerScriptingEditor",
			"Sequencer",
			"LevelSequence",
			"LevelSequenceEditor", 
			"MovieScene",
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
			"MontageGraph",
			"EditorInteractiveToolsFramework" // @todo: ~tim: why do we need this, getting a random error without it?
		});
			
	      
	}
		
}