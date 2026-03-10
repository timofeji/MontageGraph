// Created by Timofej Jermolaev, All rights reserved.

#pragma once
#include "CoreMinimal.h"
#include "SwordArcData.generated.h"

/**
 * Per-sample snapshot of the sword tip's sweep direction, baked at compile time.
 * All directions are in root-bone-local space so they are independent of world orientation.
 *
 * Stored in FMGBakedNodeData::ExtendedPayloads under the key "SwordArc".
 * Query at runtime via GetSwingDirectionAtAlpha / GetArcAngleAtAlpha using the current
 * normalised montage position (CurrentTime / TotalTime).
 */
UCLASS()
class MONTAGEGRAPH_API USwordArcData : public UObject
{
	GENERATED_BODY()

public:
	/** Normalised montage time [0,1] for each sample.  Parallel to SwingDirections / ArcAngles. */
	UPROPERTY(VisibleAnywhere, Category="SwordArc")
	TArray<float> SampleAlphas;

	/**
	 * Normalised velocity direction of the blade tip in root-local space at each sample.
	 * X = character forward, Y = character right, Z = up.
	 */
	UPROPERTY(VisibleAnywhere, Category="SwordArc")
	TArray<FVector> SwingDirections;

	/**
	 * Signed angle (degrees) of SwingDirection projected onto the root-local XY (horizontal) plane.
	 *   0   = moving directly forward
	 *  +90  = moving to the right  (standard right-to-left slash seen from front)
	 *  -90  = moving to the left
	 * ±180  = moving directly backward
	 */
	UPROPERTY(VisibleAnywhere, Category="SwordArc")
	TArray<float> ArcAngles;

	/** Normalised montage time range this data covers (start / end alpha). */
	UPROPERTY(VisibleAnywhere, Category="SwordArc")
	float RangeStart = 0.f;

	UPROPERTY(VisibleAnywhere, Category="SwordArc")
	float RangeEnd = 1.f;

	/**
	 * Return the blade-tip swing direction (root-local, normalised) at the given normalised
	 * montage time.  Linearly interpolated between the two bracketing samples.
	 */
	UFUNCTION(BlueprintCallable, Category="SwordArc")
	FVector GetSwingDirectionAtAlpha(float NormalizedAlpha) const;

	/**
	 * Return the horizontal arc angle (degrees, root-local) at the given normalised montage time.
	 * Linearly interpolated between the two bracketing samples.
	 */
	UFUNCTION(BlueprintCallable, Category="SwordArc")
	float GetArcAngleAtAlpha(float NormalizedAlpha) const;

private:
	/** Returns the lower bracket index for Alpha and the local blend fraction t in [0,1]. */
	void FindBracket(float Alpha, int32& OutLo, float& OutT) const;
};
