#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png") ), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define ICON_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)

class FMontageGraphEditorStyles : public FSlateStyleSet
{
public:
	FMontageGraphEditorStyles()
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


		Set("HBEditor.MontageGraph.Entry", new IMAGE_BRUSH("Icons/AssetIcons/MontageGraph/ActionEntry", Icon64x64));


		Set("HBEditor.Splitter", FSplitterStyle()
		                         .SetHandleNormalBrush(IMAGE_BRUSH(
			                         "Common/SplitterHandleHighlight", Icon8x8,
			                         FLinearColor(.2f, .2f, .2f, 1.f)))
		                         .SetHandleHighlightBrush(IMAGE_BRUSH(
			                         "Common/SplitterHandleHighlight", Icon8x8, FLinearColor::White))
		);

		//Graph Node Pins
		Set("HBEditor.MontageGraph.Node.Background", new IMAGE_BRUSH(
			    "Common/NodeBackground", Icon256x256));

		Set("HBEditor.MontageGraph.Node.BackgroundLight", new IMAGE_BRUSH(
			    "Common/NodeBackgroundLight", Icon256x256));

		//Graph Node Pins
		Set("HBEditor.MontageGraph.Pin.Background", new IMAGE_BRUSH(
			    "Common/PinBackground", Icon128x128));

		Set("HBEditor.MontageGraph.Pin.BackgroundHovered", new IMAGE_BRUSH(
			    "Common/PinBackgroundHovered", Icon128x128));

		Set("HBEditor.MontageGraph.Pin.BackgroundConnected", new IMAGE_BRUSH(
			    "Common/PinBackgroundConnected", Icon128x128));

		Set("HBEditor.MontageGraph.Pin.BackgroundConnectedHovered", new IMAGE_BRUSH(
			    "Common/PinBackgroundConnectedHovered", Icon128x128));


		// App Mode Icons
		Set("HBMontageGraph.AppMode.Default",
		    new IMAGE_BRUSH("Icons/AssetIcons/Blueprint_40x",
		                    Icon40x40));
		Set("HBMontageGraph.AppMode.Default.Small",
		    new IMAGE_BRUSH("Icons/AssetIcons/Blueprint_20x",
		                    Icon20x20));
		Set("HBMontageGraph.AppMode.Persona",
		    new IMAGE_BRUSH("Icons/AssetIcons/BlendSpace_40x",
		                    Icon40x40));
		Set("HBMontageGraph.AppMode.Persona.Small",
		    new IMAGE_BRUSH("Icons/AssetIcons/BlendSpace_20x",
		                    Icon20x20));

		// Toolbar Icons
		Set("HBMontageGraph.AutoArrange.Vertical",
		    new IMAGE_BRUSH("Icons/auto_arrange_vertical",
		                    Icon40x40));
		Set("HBMontageGraph.AutoArrange.Horizontal",
		    new IMAGE_BRUSH("Icons/auto_arrange_horizontal",
		                    Icon40x40));

		// Class Icons
		Set("ClassIcon.HBMontageGraph",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBMontageGraph",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_x64",
		                    Icon64x64));

		Set("ClassIcon.HBMontageGraphNodeMontage",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Montage_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBMontageGraphNodeMontage",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Montage_x64",
		                    Icon64x64));
		Set("ClassIcon.HBMontageGraphNodeMontageBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Montage_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBMontageGraphNodeMontageBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Montage_x64",
		                    Icon64x64));

		Set("ClassIcon.HBMontageGraphNodeSequence",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Sequence_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBMontageGraphNodeSequence",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Sequence_x64",
		                    Icon64x64));
		Set("ClassIcon.HBMontageGraphNodeSequenceBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Sequence_x16",
		                    Icon16x16));
		Set("ClassThumbnail.HBMontageGraphNodeSequenceBlueprint",
		    new IMAGE_BRUSH("Icons/AssetIcons/HBMontageGraph_Sequence_x64",
		                    Icon64x64));

		Set("HB.Font.Small", ICON_FONT("Fonts/NovaSquare-Regular",11));

		FSlateStyleRegistry::RegisterSlateStyle(*this);
	}


	


	static FMontageGraphEditorStyles& Get()
	{
		static FMontageGraphEditorStyles Inst;
		return Inst;
	}

	~FMontageGraphEditorStyles()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	}
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH

