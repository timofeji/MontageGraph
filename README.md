# Montage Graph Plugin 
<img width="1549" height="897" alt="UnrealEditor-Win64-DebugGame_z8RvmDM4hG" src="https://github.com/user-attachments/assets/d4b46e52-b770-4a50-84b6-e1c298b814b8" />

- [Montage Graph Plugin](#montage-graph-plugin-)
    - [Introduction](#introduction)
    - [Design](#design)
    - [Features](#features)
    - [Links](#links)
    - [Showcase](#showcase)

# Introduction
MontageGraph is an advanced animation system developed for [Eonblade](https://store.steampowered.com/app/1213840/EonBlade/). The focus of this tool is to provide new workflows for both Game Designers and Animators that speed up the authoring of structured gameplay animations. This tool should be useful if your use-case requires a more tight coupling between gameplay states and animation data. For example, if you have a lot of animations that can be arbitrarily linked together(Combos) with variable BlendSettings and fade timings, but still need finer control over certain states. Or if you're developing a game with melee combat and require precise hit detection with lag compensation.

### Design Philosophy
To achieve the performance goals of the combat system in [Eonblade](https://store.steampowered.com/app/1213840/EonBlade/),
we designed the architecture of this tool around workflows that favor compile-time caching of data that then gets interpreted by complimentary systems at runtime to produce more dynamic and interesting results. The central challenge in online melee slasher games that rely on simulated physics is having to authoritatively validate player actions based on what the player saw at the time rather than where everything is at present. As a multiplayer-first game, our primarily constraint is internet bandwidth, and as such much focus is being devoted to minimize the prediction and subsequent synchronization error between replicated pawns and their concurrent animations.



Contributions, feedback and criticism are more than welcome.
###### 🚧 Warning: this system is experimental and still under active development, expect things to break 🚧


# **Runtime Features**
- ### Collision Caching for FPS-independent evaluation:
Specifying a collision section in the timeline bakes a predefined collision shape to file using samples from the animation. When evaluating a montage through `UMontageGraphComponent`, the collisions are checked retroactively for skipped frames, ensuring consistent sweep detection with unstable framerates, similar to the [approach used by Digital Confectioners](https://www.youtube.com/watch?v=YpikjrkqNoA&t=4s)

![UnrealEditor-Win64-DebugGame_D4nU2Q1LsV](https://github.com/user-attachments/assets/c25ba3cd-884e-4948-95bc-f97a0f1d0742)

- ### Authoritative Server Rewind with [Lag Compensation](https://developer.valvesoftware.com/wiki/Lag_Compensation)
Since we're essentially storing a buffer of the past and future collision volumes anyway, we can create a client-side prediction window that is authoritatively "trusted" by the server, by specifying a trusted time interval in which we 


- ### [GameplayAbilitySystem (GAS)](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine) integration
Game designers control how to traverse the chain of dependencies in the graph, with branching montage selection based on `FGameplayTags`, GameplayQueries, and node edge adjacency supported out of the box.


* Ability to specify `UGameplayEffects` using timeline sections
 
- ### Networked AnimMontage Linking for fluid Melee/Combat Combos
- Ability to define custom Montage Blend settings for any AnimMontage Link pair








# Editor Features
- ### Live Debugging & Persona Toolkit integration
![mg_debug](https://github.com/user-attachments/assets/e5eae68c-3ce9-429f-b071-03fe0ce00afd)

- ### Custom Timeline editor
![UnrealEditor-Win64-DebugGame_SSS5CsxcXN](https://github.com/user-attachments/assets/cc41c7f1-be42-46aa-9974-dc5c7d158cfd)

The Timeline and `AnimNotify` editor has remained functionally unchanged for the past decade inside of Unreal... 
and for a good reason: it does pretty much anything you'd want it to already. However, as I was working on _yet another rewrite_ of our combat system, In parallel, I also gained a lot of interesting ideas from animating in [Cascadeur](https://cascadeur.com/). The cyclical cadence of game design, which informs artistic choices in animation, which informs system engineering, which then again informs game design, has eventually made me feel that the current paradigm limited the expression of some interesting possibilities. And so at some point I decided to explore the design space, for which the old timeline editor and `AnimNotifies` were solving for.

- ### BlendHandles
Inspired by the way clips are faded inside of most commercial [DAWs](https://en.wikipedia.org/wiki/Digital_audio_workstation), I added little handles that you can drag to adjust the fade timings inside of the default `BlendSettings`, this hopefully reduces guess work from having to manually input numbers. 

## Links
* [📚 Documentation](https://docs.eonblade.com/montagegraph)
* [📄 Source Code](https://github.com/timofeji/MontageGraph)
* [🗂️ Sample project](https://github.com/timofeji/MontageGraphSample)
* [💬 Discord](https://discord.gg/ymHY5eP)

## Showcase
projects using Montage Graph plugin:
* [EonBlade](https://store.steampowered.com/app/1213840/EonBlade/) - online sword fighting game

If you're using this plugin in one of your projects, we'd love to add you to this list 
