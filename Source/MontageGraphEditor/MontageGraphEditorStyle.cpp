// Copyright Epic Games, Inc. All Rights Reserved.

#include "MontageGraphEditorStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FMontageGraphEditorStyle::StyleInstance = nullptr;

void FMontageGraphEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMontageGraphEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FMontageGraphEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("MontageGraphEditorStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FMontageGraphEditorStyle::Create()
{
	const FVector2D Icon8x8(8.f, 8.f);
	const FVector2D Icon16x16(16.f, 16.f);
	const FVector2D Icon20x20(20.f, 20.f);
	const FVector2D Icon32x32(32.f, 32.f);
	const FVector2D Icon40x40(40.f, 40.f);
	const FVector2D Icon48x48(48.f, 48.f);
	const FVector2D Icon64x64(64.f, 64.f);
	const FVector2D Icon128x128(128.f, 128.f);
	const FVector2D Icon256x256(256.f, 256.f);


	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("MontageGraphEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("MontageGraph")->GetBaseDir() / TEXT("Resources"));


	Style->Set("ClassIcon.MontageGraph", new IMAGE_BRUSH(TEXT("Icons/Icon_MontageGraphAsset"), Icon16x16));
	Style->Set("ClassThumbnail.MontageGraph", new IMAGE_BRUSH(TEXT("Icons/Icon_MontageGraphAsset"), Icon64x64));

	//**BEGIN MontageGraph Nodes**//
	auto NodeOverlay                          = new BOX_BRUSH("Graph/Node_Title_Overlay", FMargin(1.f));
	NodeOverlay->DrawAs                       = ESlateBrushDrawType::RoundedBox;
	NodeOverlay->OutlineSettings.Width        = 5.f;
	NodeOverlay->OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	NodeOverlay->OutlineSettings.CornerRadii  = FVector4(6.f, 6.f, 3.f, 3.f);
	NodeOverlay->Tiling                       = ESlateBrushTileType::Both;


	auto NodeOverlayBody    = new IMAGE_BRUSH(TEXT("Graph/Node_Title_Overlay_Body"), CoreStyleConstants::Icon64x64);
	NodeOverlayBody->Margin = FMargin(5.f, 5.f);
	NodeOverlayBody->DrawAs = ESlateBrushDrawType::Image;
	NodeOverlayBody->Tiling = ESlateBrushTileType::Horizontal;

	Style->Set("MontageGraph.Node.Title.Overlay", NodeOverlay);
	Style->Set("MontageGraph.Node.Title.Overlay.Body", NodeOverlayBody);

	Style->Set("MontageGraph.Node.Body", new BOX_BRUSH("Graph/Node_Body", FMargin(16.f/64.f)));
	Style->Set("MontageGraph.Node.Shadow", new BORDER_BRUSH("Graph/Node_Shadow", FMargin(.5f)));
	Style->Set("MontageGraph.Node.ActiveShadow", new BORDER_BRUSH("Graph/Node_Shadow_Active", FMargin(.5f)));
	Style->Set("MontageGraph.Node.WasActiveShadow", new BOX_BRUSH("Graph/Node_Shadow_WasActive", FMargin(18.0f/64.0f)));


	Style->Set("MontageGraph.Node.Icon.Selector", new IMAGE_BRUSH("Icons/Icon_Node_Selector", Icon20x20));


	Style->Set("MontageGraph.Icon.Tracers", new IMAGE_BRUSH("Icons/Icon_RegenerateTracers", Icon16x16));

	Style->Set("MontageGraph.Icon.NodeBlends", new IMAGE_BRUSH("Icons/Icon_NodeBlends", Icon40x40));
	//**END MontageGraph Nodes**//
	//
	Style->Set("Graph.Node.Body", new BOX_BRUSH("Graph/Node_Body",
	                                            FMargin(16.f/64.f, 25.f/64.f, 16.f/64.f, 16.f/64.f)));
	Style->Set("Graph.Node.ColorSpill",
	           new BOX_BRUSH("Graph/Node_ColorSpill", FMargin(4.0f/64.0f, 4.0f/32.0f)));
	//
	// Set("Graph.Node.Icon", new IMAGE_BRUSH_SVG("Starship/AnimationBlueprintEditor/AnimationState", Icon16x16));
	// Set("Graph.ConduitNode.Icon", new IMAGE_BRUSH_SVG("Starship/AnimationBlueprintEditor/AnimationConduit", Icon16x16));
	// Set("Graph.AliasNode.Icon", new IMAGE_BRUSH_SVG("Starship/AnimationBlueprintEditor/AnimationAlias", Icon16x16));
	//

	Style->Set("Graph.Node.Pin.BackgroundHovered", new BOX_BRUSH("Graph/Node_Pin_HoverCue",
	                                                             FMargin(12.0f/64.0f,12.0f/64.0f,12.0f/64.0f,12.0f/64.0f
	                                                             )));
	Style->Set("Graph.Node.Pin.Background", new FSlateNoResource());


	Style->Set("Graph.Link.Icon", new IMAGE_BRUSH(TEXT("Icons/Icon_Bind"), Icon40x40));
	Style->Set("Graph.Node.Icon.Bind", new IMAGE_BRUSH(TEXT("Icons/Icon_Bind"), Icon16x16));
	Style->Set("Graph.Link.IconSmall", new IMAGE_BRUSH(TEXT("Icons/Icon_Link_Small"), Icon16x16));


	//**BEGIN MontageGraph DopeSheet**//
	Style->Set("MontageGraph.DopeSheet.Cell", new BORDER_BRUSH("Timeline/DopeSheet_Cell", FMargin(2.0f/32.0f)));
	Style->Set("MontageGraph.DopeSheet.Cell.Hover", new IMAGE_BRUSH("Timeline/DopeSheet_Cell_Hover", Icon40x40));
	Style->Set("MontageGraph.DopeSheet.CellAlt", new IMAGE_BRUSH("Timeline/DopeSheet_CellAlt", Icon40x40));
	
	Style->Set("MontageGraph.DopeSheet.Border.Selecting",
	           new BORDER_BRUSH("Timeline/DopeSheet_Border_Selecting", FMargin(6.0f/32.0f)));
	Style->Set("MontageGraph.DopeSheet.Border.Selected",
	           new BORDER_BRUSH("Timeline/DopeSheet_Border_Selected", FMargin(6.0f/32.0f)));
	
	Style->Set("MontageGraph.SequenceTrack.Section.Border",
	           new BORDER_BRUSH("Timeline/TimelineTrack_Sequence_Section_Border", FMargin(6.0f/32.0f)));

	Style->Set("MontageGraph.SequenceTrack.Section.Preview",
	           new BORDER_BRUSH("Timeline/TimelineTrack_DashedBorder", FMargin(6.0f/32.0f)));
	

	Style->Set("MontageGraph.SequenceTrack.Section.ResizeHandle",
	           new IMAGE_BRUSH("Timeline/TimelineSection_ResizeHandle", Icon32x32));
	Style->Set("MontageGraph.SequenceTrack.Section.ResizeHandle.Hovered",
	           new IMAGE_BRUSH("Timeline/TimelineSection_ResizeHandle_Hovered", Icon32x32));
	
	
	Style->Set("MontageGraph.SequenceTrack.Section.Body",
	           new IMAGE_BRUSH("Timeline/TimelineTrack_Sequence_Section_Body", Icon40x40));
	Style->Set("MontageGraph.SequenceTrack.Section.Body.Hovered",
	           new IMAGE_BRUSH("Timeline/TimelineTrack_Sequence_Section_Hovered", Icon40x40));

	Style->Set("Graph.Node.Pin.BackgroundHovered", new BOX_BRUSH("Graph/Node_Pin_HoverCue", FMargin(12.0f/64.0f)));
	//**END MontageGraph DopeSheet**//

	//**BEGIN MontageGraph Timeline**//
	Style->Set("MontageGraph.Timeline.PlayHead", new IMAGE_BRUSH("Timeline/Timeline_Playhead", Icon40x40));
	Style->Set("MontageGraph.Timeline.PlayHead.Preview",
	           new IMAGE_BRUSH("Timeline/Timeline_Playhead_Preview", Icon40x40));

	Style->Set("MontageGraph.TimelineSlider.Background",
	           new IMAGE_BRUSH("Timeline/Timeline_Background", FVector2D(32.f/2, 32.f )));
	Style->Set("MontageGraph.TimelineSlider.Background.Inactive",
	           new IMAGE_BRUSH("Timeline/Timeline_Background_Inactive", FVector2D(32.f/2, 32.f )));
	
	
	Style->Set("MontageGraph.TimelineSlider.Cell.Major", new IMAGE_BRUSH("Timeline/Timeline_Cell_Major", Icon40x40));
	Style->Set("MontageGraph.TimelineSlider.Cell.Start", new IMAGE_BRUSH("Timeline/Timeline_Cell_Start", Icon40x40));
	Style->Set("MontageGraph.TimelineSlider.Cell.End", new IMAGE_BRUSH("Timeline/Timeline_Cell_End", Icon40x40));

	Style->Set("MontageGraph.TimelineSlider.Void", new IMAGE_BRUSH("Timeline/Timeline_Void",
	                                                               FVector2D(64.f,32.f), FStyleColors::Background,
	                                                               ESlateBrushTileType::Both));
	
	
	//**END MontageGraph Timeline**//


	return Style;
}

void FMontageGraphEditorStyle::ReloadTextures()
{
	// if (FSlateApplication::IsInitialized())
	// {
	// 	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	// }
}

const ISlateStyle& FMontageGraphEditorStyle::Get()
{
	return *StyleInstance;
}
