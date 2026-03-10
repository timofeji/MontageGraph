// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"
#include "UObject/Object.h"
#include "MontageTrack_BlendLinks.generated.h"




class UMGEdNode_Montage;

class SMGBlendLinksTrack : public SDopeSheetTrackTimeline
{
public:
	SLATE_BEGIN_ARGS(SMGBlendLinksTrack)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr);

	virtual void MakeTrackContextMenu(FMenuBuilder& ContextMenuBuilder) override;
};



UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_LinkBlend : public UDopeSheetTrackSection
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Blends")
	FMontageGraphLinkSettings LinkSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link Blends")
	UMGEdNode_Montage* TargetNode = nullptr;

};


/**
 * 
 */

#if WITH_EDITOR
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_BlendLinks : public UDopeSheetTrackBase
{
	GENERATED_BODY()
	
	UMontageTrack_BlendLinks (const FObjectInitializer& ObjectInitializer);
	
	// virtual UObject* GenerateNewDataAsset(UObject* Outer, FName Name) override;

	/*Begin IDopeSheetTrack Interface*/
	virtual const FText         GetTrackName() const override;
	virtual const FLinearColor  GetTrackColor() const override;
	virtual const FLinearColor  GetSectionColor() const override;
	virtual FSlateIcon          GetTrackIcon() const override;
	
	/*End IDopeSheetTrack Interface*/

	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(
		TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const override;

	virtual void KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame) override;
	
	void AddLink();

};
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_LinkBlendDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new UMontageTrackSection_LinkBlendDetails);
	}


	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// Called when a new item is selected in the combo box
	void OnTargetNodeChanged(UMGEdNode_Montage* NewSelection, ESelectInfo::Type SelectInfo);

	// Returns the display name for the current combo box selection
	FText GetCurrentTargetNodeText() const;

	// Builds the combo box widget
	TSharedRef<SWidget> MakeNodeWidget(UMGEdNode_Montage* InNode);

	TArray<UMGEdNode_Montage*>      NodeOptions;
	UMontageTrackSection_LinkBlend* EditedSection;
};
#endif
