#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png") ), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define ICON_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)

class FActionGraphEditorStyles : public FSlateStyleSet
{
public:
	FActionGraphEditorStyles()
		: FSlateStyleSet("HyperbladeEditorStyle")
	{
		const FVector2D Icon8x8(8.f,
		                        8.f);
		const FVector2D Icon16x16(16.f,
		                          16.f);
		const FVector2D Icon20x20(20.f,
		                          20.f);
		const FVector2D Icon40x40(40.f,
		                          40.f);
		const FVector2D Icon64x64(64.f,
		                          64.f);
		const FVector2D Icon128x128(128.f,
		                            128.f);
		
		const FVector2D Icon256x256(256.f,
		                            256.f);


		SetContentRoot(FPaths::ProjectDir() / TEXT("Resources"));

		/** Gets the style name. */

		Set("HBEditor.HBUser.SpaceRock",
		    new IMAGE_BRUSH("Icons/AssetIcons/Wallet_Icon_SpaceRock",
		                    Icon16x16));
		Set("HBEditor.HBUser.TimeOre",
		    new IMAGE_BRUSH("Icons/AssetIcons/Wallet_Icon_TimeOre",
		                    Icon16x16));

		Set("HBEditor.HBUser.HBIcon",
		    new IMAGE_BRUSH("Icons/AssetIcons/Icon_HB_Logo",
		                    Icon64x64));
		
		
		Set("HBEditor.Character.WeaponAction", new IMAGE_BRUSH
		("Icons/AssetIcons/Icon_CharacterAction", Icon20x20));


		Set("HBEditor.ActionGraph.Entry", new IMAGE_BRUSH("Icons/AssetIcons/ActionGraph/ActionEntry", Icon64x64));


		Set("HBEditor.Splitter", FSplitterStyle()
		                         .SetHandleNormalBrush(IMAGE_BRUSH(
			                         "Common/SplitterHandleHighlight", Icon8x8,
			                         FLinearColor(.2f, .2f, .2f, 1.f)))
		                         .SetHandleHighlightBrush(IMAGE_BRUSH(
			                         "Common/SplitterHandleHighlight", Icon8x8, FLinearColor::White))
		);

		//Graph Node Pins
		Set("HBEditor.ActionGraph.Node.Background", new IMAGE_BRUSH(
			    "Common/NodeBackground", Icon256x256));

		Set("HBEditor.ActionGraph.Node.BackgroundLight", new IMAGE_BRUSH(
			    "Common/NodeBackgroundLight", Icon256x256));

		//Graph Node Pins
		Set("HBEditor.ActionGraph.Pin.Background", new IMAGE_BRUSH(
			    "Common/PinBackground", Icon128x128));

		Set("HBEditor.ActionGraph.Pin.BackgroundHovered", new IMAGE_BRUSH(
			    "Common/PinBackgroundHovered", Icon128x128));

		Set("HBEditor.ActionGraph.Pin.BackgroundConnected", new IMAGE_BRUSH(
			    "Common/PinBackgroundConnected", Icon128x128));

		Set("HBEditor.ActionGraph.Pin.BackgroundConnectedHovered", new IMAGE_BRUSH(
			    "Common/PinBackgroundConnectedHovered", Icon128x128));


		// App Mode Icons
		Set("HBActionGraph.AppMode.Default",
		    new IMAGE_BRUSH("Icons/AssetIcons/Blueprint_40x",
		                    Icon40x40));
		Set("HBActionGraph.AppMode.Default.Small",
		    new IMAGE_BRUSH("Icons/AssetIcons/Blueprint_20x",
		                    Icon20x20));
		Set("HBActionGraph.AppMode.Persona",
		    new IMAGE_BRUSH("Icons/AssetIcons/BlendSpace_40x",
		                    Icon40x40));
		Set("HBActionGraph.AppMode.Persona.Small",
		    new IMAGE_BRUSH("Icons/AssetIcons/BlendSpace_20x",
		                    Icon20x20));

		// Toolbar Icons
		Set("HBActionGraph.AutoArrange.Vertical",
		    new IMAGE_BRUSH("Icons/auto_arrange_vertical",
		                    Icon40x40));
		Set("HBActionGraph.AutoArrange.Horizontal",
		    new IMAGE_BRUSH("Icons/auto_arrange_horizontal",
		                    Icon40x40));

		// Class Icons
		Set("ClassIcon.HBActionGraph",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBActionGraph",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_x64",
		                    Icon64x64));

		Set("ClassIcon.HBActionGraphNodeMontage",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Montage_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBActionGraphNodeMontage",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Montage_x64",
		                    Icon64x64));
		Set("ClassIcon.HBActionGraphNodeMontageBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Montage_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBActionGraphNodeMontageBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Montage_x64",
		                    Icon64x64));

		Set("ClassIcon.HBActionGraphNodeSequence",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Sequence_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBActionGraphNodeSequence",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Sequence_x64",
		                    Icon64x64));
		Set("ClassIcon.HBActionGraphNodeSequenceBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Sequence_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBActionGraphNodeSequenceBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBActionGraph_Sequence_x64",
		                    Icon64x64));

		Set("HB.Font.Small", ICON_FONT("Fonts/NovaSquare-Regular",11));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}


	


	static FActionGraphEditorStyles& Get()
	{
		static FActionGraphEditorStyles Inst;
		return Inst;
	}

	~FActionGraphEditorStyles()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH

