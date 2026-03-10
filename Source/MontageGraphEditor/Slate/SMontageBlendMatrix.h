// Copyright Drop Games Inc.

#pragma once

#include "CoreMinimal.h"
#include "MontageGraph/MontageGraph.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class MONTAGEGRAPHEDITOR_API SMontageBlendMatrix : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMontageBlendMatrix)
		{
		}
		SLATE_ARGUMENT(UMontageGraph*, Graph)

	SLATE_END_ARGS()


	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	UMontageGraph*  Graph;
};
