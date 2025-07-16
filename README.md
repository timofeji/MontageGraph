# Montage Graph Plugin 
<img width="1929" height="1089" alt="UnrealEditor-Win64-DebugGame_vTOvhIHpV4" src="https://github.com/user-attachments/assets/297dfa05-bf69-45b4-bc27-f9005b9cb1cf" />

- [Montage Graph Plugin](#montage-graph-plugin-)
    - [About](#about)
    - [Features](#features)
    - [Links](#links)
    - [Showcase](#showcase)


## About
Montage Graph is an experimental animation authoring system developed for [Eonblade](https://store.steampowered.com/app/1213840/EonBlade/).
The focus of this tool is to speed up authoring of structured gameplay animations. This tool should be useful if your use-case includes animations that can be linked together(Combos), you require the ability to blend arbitrary AnimMontages with custom BlendSettings, or if you're developing a game with melee combat and require precise hit detection with lag compensation.

Contributions, feedback and criticism are more than welcome. 

## Features
- Runtime:
  - First-Class integration with [Gameplay Ability System(GAS)](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine)
  - Collision Caching for Authoritative Runtime [Lag Compensation](https://developer.valvesoftware.com/wiki/Lag_Compensation)
  - Networked AnimMontage Linking for fluid Melee/Combat Combos
  - Ability to define custom Montage Blend settings for any AnimMontage Link pair
- Graph Editor:
    - Branching montage selection based GameplayTags and selector nodes
- DopeSheet:
    

## Links
* [📚 Documentation](https://docs.eonblade.com/montagegraph) 
* [📄 Source Code](https://github.com/timofeji/MontageGraph)
* [🗂️ Sample project](https://github.com/timofeji/MontageGraphSample)
* [💬 Discord](https://discord.gg/ymHY5eP)

## Showcase
projects using Montage Graph plugin:
* [EonBlade](https://store.steampowered.com/app/1213840/EonBlade/) - online sword fighting game 

If you're using this plugin in one of your projects, we'd love to add you to this list 

