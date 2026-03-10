// Copyright Drop Games Inc.

#include "SMontageBlendMatrix.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMontageBlendMatrix::Construct(const FArguments& InArgs)
{
	Graph = InArgs._Graph;
	
	
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox)
	.Orientation(Orient_Horizontal)
	+ SScrollBox::Slot()
	[
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		[
			// Primary grid
			SNew(SBorder)
			.Padding(4)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					Grid
				]
			]
		]
	];

	Grid->AddSlot(0, 0)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock).Text(FText::FromString(TEXT("B")))
	];

	int32 N = Graph->Montages.Num();
	for (int32 Col = 0; Col < N; ++Col)
	{
		FString HeaderName = Graph->Montages[Col] ? Graph->Montages[Col]->GetName() : FString::Printf(TEXT("Node_%d"), Col);
		Grid->AddSlot(Col + 1, 0)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(HeaderName))
			.AutoWrapText(true)
		];
	}

	// Row headers + cells
	for (int32 Row = 0; Row < N; ++Row)
	{
		// Row header at (0, Row+1)
		FString RowName = Graph->Montages[Row] ? Graph->Montages[Row]->GetName() : FString::Printf(TEXT("Node_%d"), Row);
		Grid->AddSlot(0, Row + 1)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(RowName))
			.AutoWrapText(true)
		];

		// Cells columns
		for (int32 Col = 0; Col < N; ++Col)
		{
			// Optionally disable editing lower triangle if symmetric mode is on.
			const bool bIsLowerTriangle = (Col < Row);
			const bool bCellEnabled = true;

			// Create the checkbox for this cell
			TSharedRef<SCheckBox> Check = SNew(SCheckBox)
				// .IsChecked_Lambda([this, Row, Col, bHasGetter]() -> ECheckBoxState
				// {
				// 	if (!bHasGetter) return ECheckBoxState::Undetermined;
				// 	const bool bPresent = OnGetEdge.Execute(Row, Col);
				// 	return bPresent ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				// })
				// .OnCheckStateChanged_Lambda([this, Row, Col, bHasSetter, bSymmetric](ECheckBoxState NewState)
				// {
				// 	if (!bHasSetter) return;
				// 	const bool bNew = (NewState == ECheckBoxState::Checked);
				// 	OnSetEdge.Execute(Row, Col, bNew);
				//
				// 	// If symmetric/undirected, mirror to (Col,Row)
				// 	if (bSymmetric && (Row != Col))
				// 	{
				// 		OnSetEdge.Execute(Col, Row, bNew);
				// 	}
				// })
				.IsEnabled(bCellEnabled);

			// Add cell to grid at (Col+1, Row+1)
			Grid->AddSlot(Col + 1, Row + 1)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(2)
			[
				Check
			];
		}
	}

	ChildSlot
	[
		ScrollBox
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION