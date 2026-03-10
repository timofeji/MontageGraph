#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"
#include "MontageTrack_GameplayState.generated.h"

#if WITH_EDITOR

// ============================================================
// Section data — duration bar that grants loose tags
// ============================================================

UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_GameplayState : public UDopeSheetTrackSection
{
	GENERATED_BODY()
public:
	/** Tags added loosely to the owner's ASC for the duration of this section. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayTagContainer LooseTags;
};

// ============================================================
// Custom timeline widget — standard bars with tag context menu
// ============================================================

class MONTAGEGRAPHEDITOR_API SMGGameplayStateTrack : public SDopeSheetTrackTimeline
{
public:
	SLATE_BEGIN_ARGS(SMGGameplayStateTrack) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr);

	virtual void MakeTrackContextMenu(FMenuBuilder& MenuBuilder) override;
	virtual void MakeSectionContextMenu(FMenuBuilder& MenuBuilder) override;

private:
	double LastRightClickTime = 0.0;
};

// ============================================================
// Track class
// ============================================================

UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_GameplayState : public UDopeSheetTrackBase
{
	GENERATED_BODY()
public:
	UMontageTrack_GameplayState();

	virtual void KeyTimeRange(float SelectionStartTime, float SelectionEndTime) override;

	virtual void BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
	                        UMontageGraph* OwnerGraph, const FString& DisplayName) override;

	virtual const FText        GetTrackName()    const override;
	virtual const FLinearColor GetTrackColor()   const override;
	virtual const FLinearColor GetSectionColor() const override;
	virtual FSlateIcon         GetTrackIcon()    const override;

	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(
		TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const override;
};

#endif
