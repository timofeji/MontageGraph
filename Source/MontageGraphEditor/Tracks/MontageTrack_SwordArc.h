// Created by Timofej Jermolaev, All rights reserved.

#pragma once
#include "CoreMinimal.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "MontageTrack_SwordArc.generated.h"

class UMontageGraph;

#if WITH_EDITOR

/**
 * Section placed on the SwordArc track that defines the active attack window and
 * the bones used to measure the blade tip's sweep direction.
 */
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_SwordArc : public UDopeSheetTrackSection
{
	GENERATED_BODY()

public:
	/**
	 * The blade tip (or furthest point) bone / socket to track.
	 * The velocity of this bone in root-local space defines the arc direction.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SwordArc")
	FName TipSocket = FName("weapon_tip");

	/**
	 * Root / pelvis bone used to establish the character-local reference frame.
	 * The tip position is expressed relative to this bone so direction data is
	 * independent of world translation and root-motion drift.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SwordArc")
	FName RootSocket = FName("root");

	/**
	 * How many arc samples to bake per animation frame.
	 * Higher = finer direction data; 1.0 = one sample per frame.
	 * Typically 1–2 is sufficient (arc direction changes slowly).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SwordArc", meta=(ClampMin="0.25", ClampMax="4.0"))
	double SampleMultiplier = 1.0;
};


/**
 * Dope-sheet track that bakes sword-arc data into USwordArcData and writes it to
 * FMGBakedNodeData::ExtendedPayloads["SwordArc"].
 *
 * Bake algorithm (per section):
 *   1. Sample bone poses at (1/SampleMultiplier) × frame-interval steps.
 *   2. For each adjacent pose pair, compute the blade tip's velocity vector in
 *      root-local space: delta_tip_root = (NextTip - NextRoot) - (CurTip - CurRoot).
 *   3. Normalise → SwingDirection.  Project onto local XY plane → ArcAngle (degrees).
 *   4. Assign to the midpoint normalised time between the two frames.
 *
 * Runtime cost: one binary-search + one lerp per query — effectively free.
 */
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_SwordArc : public UDopeSheetTrackBase
{
	GENERATED_BODY()

public:
	UMontageTrack_SwordArc(const FObjectInitializer& ObjectInitializer);

	virtual UObject* GenerateNewDataAsset(UObject* Outer, FName Name) override;

	virtual void BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
	                        UMontageGraph* OwnerGraph, const FString& DisplayName) override;

	virtual void KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame) override;

	/*~ IDopeSheetTrack interface */
	virtual const FText        GetTrackName()  const override;
	virtual const FLinearColor GetTrackColor() const override;
	virtual FSlateIcon         GetTrackIcon()  const override;
	/*~ End IDopeSheetTrack interface */

	/** Key used to store / retrieve USwordArcData in FMGBakedNodeData::ExtendedPayloads. */
	static const FName PayloadKey;
};

#endif // WITH_EDITOR
