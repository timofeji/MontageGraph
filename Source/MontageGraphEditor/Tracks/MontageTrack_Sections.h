#pragma once

#include "CoreMinimal.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"
#include "MontageTrack_Sections.generated.h"

#if WITH_EDITOR

/** A single named montage section marker placed at a specific time. */
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_SectionName : public UDopeSheetTrackSection
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName SectionName = FName("Default");
};


class SMGSectionsTrack : public SDopeSheetTrackTimeline
{
public:
	SLATE_BEGIN_ARGS(SMGSectionsTrack) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                      const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	                      int32 InLayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void MakeTrackContextMenu(FMenuBuilder& MenuBuilder) override;
	virtual void MakeSectionContextMenu(FMenuBuilder& MenuBuilder) override;

private:
	/** Returns the index of the section marker under LocalPos, or -1. */
	int32 HitTestMarker(const FGeometry& Geometry, const FVector2D& LocalPos) const;

	double LastRightClickTime    = 0.0;
	int32  ContextSectionIndex   = -1;
};


/** Track that holds named montage section markers and bakes them into the montage. */
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_Sections : public UDopeSheetTrackBase
{
	GENERATED_BODY()
public:
	/**
	 * Replaces CompositeSections on an already-built montage with the markers
	 * defined on this track.  Called from UMontageTrack_Sequences::BakeToNode.
	 */
	void ApplyToMontage(UAnimMontage* Montage) const;

	virtual const FText        GetTrackName() const override;
	virtual const FLinearColor GetTrackColor() const override;
	virtual const FLinearColor GetSectionColor() const override;
	virtual FSlateIcon         GetTrackIcon() const override;

	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(
		TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const override;
};

#endif
