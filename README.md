# Montage Graph Editor 

<img width="1549" height="897" alt="UnrealEditor-Win64-DebugGame_z8RvmDM4hG" src="https://github.com/user-attachments/assets/d4b46e52-b770-4a50-84b6-e1c298b814b8" />

- [Montage Graph Plugin](#montage-graph-plugin-)
    - [About](#about)
    - [Features](#features)
    - [Links](#links)
    - [Showcase](#showcase)


## About
Montage Graph is an experimental animation authoring system developed for [Eonblade](https://store.steampowered.com/app/1213840/EonBlade/).
The focus of this tool is to speed up authoring of structured gameplay animations. This tool should be useful if your use-case includes animations that can be linked together(Combos), you require the ability to blend arbitrary AnimMontages with custom BlendSettings, or if you're developing a game with melee combat and require precise hit detection with lag compensation.

Contributions, feedback and criticism are more than welcome. 

## Runtime Features
  - First-Class integration with [Gameplay Ability System(GAS)](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine)
  - Collision Caching for Authoritative Runtime [Lag Compensation](https://developer.valvesoftware.com/wiki/Lag_Compensation)
  - Networked AnimMontage Linking for fluid Melee/Combat Combos
  - Ability to define custom Montage Blend settings for any AnimMontage Link pair
  - Branching montage selection based on `FGameplayTags` and node edges
  - Ability to specify `UGameplayEffects` using timeline sections
  - Persona Toolkit integration
    
### Collision Caching with FPS-independent evaluation 
![UnrealEditor-Win64-DebugGame_D4nU2Q1LsV](https://github.com/user-attachments/assets/c25ba3cd-884e-4948-95bc-f97a0f1d0742)

Specifying a collision section in the timeline bakes a predefined collision shape to file using samples from the animation. When evaluating a montage through `UMontageGraphComponent`, the collisions are checked retroactively for skipped frames, ensuring consistent sweep detection with unstable framerates, similar to the [approach used by Digital Confectioners](https://www.youtube.com/watch?v=YpikjrkqNoA&t=4s)

### Custom Timeline editor
![UnrealEditor-Win64-DebugGame_SSS5CsxcXN](https://github.com/user-attachments/assets/cc41c7f1-be42-46aa-9974-dc5c7d158cfd)





## Links
* [📚 Documentation](https://docs.eonblade.com/montagegraph) 
* [📄 Source Code](https://github.com/timofeji/MontageGraph)
* [🗂️ Sample project](https://github.com/timofeji/MontageGraphSample)
* [💬 Discord](https://discord.gg/ymHY5eP)

## Showcase
projects using Montage Graph plugin:
* [EonBlade](https://store.steampowered.com/app/1213840/EonBlade/) - online sword fighting game 

If you're using this plugin in one of your projects, we'd love to add you to this list 

