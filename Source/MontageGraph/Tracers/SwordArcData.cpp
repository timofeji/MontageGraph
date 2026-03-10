// Created by Timofej Jermolaev, All rights reserved.
#include "SwordArcData.h"

void USwordArcData::FindBracket(float Alpha, int32& OutLo, float& OutT) const
{
	const int32 N = SampleAlphas.Num();

	if (Alpha <= SampleAlphas[0])
	{
		OutLo = 0;
		OutT  = 0.f;
		return;
	}
	if (Alpha >= SampleAlphas[N - 1])
	{
		OutLo = N - 2;
		OutT  = 1.f;
		return;
	}

	// Binary search for the lower-bracket index.
	int32 Lo = 0, Hi = N - 1;
	while (Hi - Lo > 1)
	{
		const int32 Mid = (Lo + Hi) / 2;
		if (SampleAlphas[Mid] <= Alpha) Lo = Mid;
		else                            Hi = Mid;
	}

	const float Span = SampleAlphas[Hi] - SampleAlphas[Lo];
	OutLo = Lo;
	OutT  = (Span > SMALL_NUMBER) ? ((Alpha - SampleAlphas[Lo]) / Span) : 0.f;
}

FVector USwordArcData::GetSwingDirectionAtAlpha(float NormalizedAlpha) const
{
	if (SampleAlphas.Num() < 2)
	{
		return SwingDirections.Num() > 0 ? SwingDirections[0] : FVector::ForwardVector;
	}

	int32 Lo; float T;
	FindBracket(NormalizedAlpha, Lo, T);

	return FMath::Lerp(SwingDirections[Lo], SwingDirections[Lo + 1], T).GetSafeNormal();
}

float USwordArcData::GetArcAngleAtAlpha(float NormalizedAlpha) const
{
	if (SampleAlphas.Num() < 2)
	{
		return ArcAngles.Num() > 0 ? ArcAngles[0] : 0.f;
	}

	int32 Lo; float T;
	FindBracket(NormalizedAlpha, Lo, T);

	return FMath::Lerp(ArcAngles[Lo], ArcAngles[Lo + 1], T);
}
