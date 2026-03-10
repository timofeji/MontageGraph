#include "MontageTrack_Sequences.h"

#include "AlphaBlend.h"
#include "AssetSelection.h"
#include "MontageGraph/MontageGraph.h"
#include "AssetToolsModule.h"
#include "MontageGraphEditorLog.h"
#include "MontageGraphEditorStyle.h"
#include "MontageTrack_BlendLinks.h"
#include "MontageTrack_Sections.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Graph/EdNodes/MGEdNode_Montage.h"
#include "Slate/SAnimMontageSlotPicker.h"

namespace FDopeSheetStyle
{
	static const FVector2D BlendHandleSize = FVector2D(10.f, 10.f);
	static const FLinearColor AnimSequenceColor = FColor(80, 123, 72, 255);

	static const FSlateBrush* SectionBodyBrush         = nullptr;
	static const FSlateBrush* SectionBodyBrush_Hovered = nullptr;
	static const FSlateBrush* SectionBorderBrush       = nullptr;
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "MontageTrack_Sequences"
TSharedPtr<SWidget> FDopeSheetSequenceSectionDragDrop::GetDefaultDecorator() const
{
	return SectionBeingDragged.Pin();
}

void FDopeSheetSequenceSectionDragDrop::Construct()
{
	FDragDropOperation::Construct();

	CursorDecoratorWindow->SetOpacity(.25f);
}

void FDopeSheetSequenceSectionDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	// if ( bDropWasHandled == false )
	// {
	// 	if (SectionBeingDragged.IsValid())
	// 	{
	// 		SectionBeingDragged.Pin()->OnDropCancelled(MouseEvent);
	// 	}
	// }
	//

	if (OwningTrack.IsValid())
	{
		OwningTrack.Pin()->OnRearrangeDrop();
	}


	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}


void FDopeSheetSequenceSectionDragDrop::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	FSlateRect BoundingRect = OwningTrack.Pin()->GetCachedGeometry().GetLayoutBoundingRect();
	FSlateRect SectionRect  = SectionBeingDragged.Pin()->GetCachedGeometry().GetLayoutBoundingRect();


	FVector2D MousePos = DragDropEvent.GetScreenSpacePosition();
	FVector2D SectionDerotatowPos;
	SectionDerotatowPos.X = FMath::Clamp((MousePos + Offset).X,
	                                     BoundingRect.Left,
	                                     BoundingRect.Right - SectionRect.GetSize().X);
	SectionDerotatowPos.Y = StartingScreenPos.Y;
	CursorDecoratorWindow->MoveWindowTo(SectionDerotatowPos);

	OwningTrack.Pin()->PreviewRearrange(MousePos);
}

TSharedRef<FDopeSheetSequenceSectionDragDrop> FDopeSheetSequenceSectionDragDrop::New(
	TSharedRef<SMGSequenceTrack> ParentTrack,
	TSharedRef<SWidget>          SectionWidget, const FVector2D& CursorPosition, const FVector2D& ScreenPositionOfNode)
{
	TSharedRef<FDopeSheetSequenceSectionDragDrop> Operation = MakeShareable(new FDopeSheetSequenceSectionDragDrop);

	Operation->OwningTrack         = ParentTrack;
	Operation->SectionBeingDragged = SectionWidget;
	Operation->Offset              = ScreenPositionOfNode - CursorPosition;
	Operation->StartingScreenPos   = ScreenPositionOfNode;

	Operation->Construct();


	return Operation;
}

// ============================================================
// FMGBlendHandleDragDrop
// ============================================================

TSharedPtr<SWidget> FMGBlendHandleDragDrop::GetDefaultDecorator() const
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				UMontageTrackSection_Sequences* Data = SectionDataPtr.Get();
				if (!Data) return FText::GetEmpty();
				const float T = HandleType == EMGBlendHandleType::BlendIn
					? Data->BlendInSettings.Blend.BlendTime
					: Data->BlendOutSettings.Blend.BlendTime;
				return FText::FromString(FString::Printf(TEXT("%s: %.2fs"),
					HandleType == EMGBlendHandleType::BlendIn ? TEXT("Blend In") : TEXT("Blend Out"), T));
			})
		];
}

void FMGBlendHandleDragDrop::Construct()
{
	FDragDropOperation::Construct();
}

void FMGBlendHandleDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}

void FMGBlendHandleDragDrop::OnDragged(const FDragDropEvent& DragDropEvent)
{
	UMontageTrackSection_Sequences* Data = SectionDataPtr.Get();
	if (!Data || !ControllerPtr) return;

	const FVector2D MousePos  = DragDropEvent.GetScreenSpacePosition();
	const float     MouseTime = static_cast<float>(ControllerPtr->AbsoluteXCoordToTime(MousePos.X));
	const float     HalfDur   = static_cast<float>(Data->EndTime - Data->StartTime) * 0.5f;

	if (HandleType == EMGBlendHandleType::BlendIn)
	{
		// Blend-in time = distance from section start to cursor.
		Data->BlendInSettings.Blend.BlendTime =
			FMath::Clamp(MouseTime - static_cast<float>(Data->StartTime), 0.f, HalfDur);
	}
	else
	{
		// Blend-out time = distance from cursor to section end.
		Data->BlendOutSettings.Blend.BlendTime =
			FMath::Clamp(static_cast<float>(Data->EndTime) - MouseTime, 0.f, HalfDur);
	}

	CursorDecoratorWindow->MoveWindowTo(MousePos + FVector2D(14.f, -14.f));
}

TSharedRef<FMGBlendHandleDragDrop> FMGBlendHandleDragDrop::New(
	TWeakObjectPtr<UMontageTrackSection_Sequences> InSectionData,
	TSharedPtr<FDopeSheetController>               InController,
	EMGBlendHandleType                             InHandleType)
{
	TSharedRef<FMGBlendHandleDragDrop> Op = MakeShareable(new FMGBlendHandleDragDrop);
	Op->SectionDataPtr = InSectionData;
	Op->ControllerPtr  = InController;
	Op->HandleType     = InHandleType;

	if (UMontageTrackSection_Sequences* Data = InSectionData.Get())
	{
		Data->Modify();
	}

	Op->Construct();
	return Op;
}

// ============================================================
// SMGSequenceBlendHandle
// ============================================================

void SMGSequenceBlendHandle::Construct(const FArguments& InArgs)
{
	SectionDataPtr = InArgs._SectionData;
	ControllerPtr  = InArgs._Controller;
	HandleType     = InArgs._HandleType;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(1.f)
		.Content()
		[
			SNew(SImage)
			.Image_Lambda([this] {
				return HandleType == EMGBlendHandleType::BlendIn
						   ? FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.SequenceTrack.BlendHandle.In")
						   : FMontageGraphEditorStyle::Get().GetBrush("MontageGraph.SequenceTrack.BlendHandle.Out");
			})
			.DesiredSizeOverride(FDopeSheetStyle::BlendHandleSize)
			.ColorAndOpacity_Lambda([this]() {
				const float Alpha = bIsHovered ? 1.f : .72f;
				return FLinearColor(0.3f, 0.85f, 0.5f, Alpha);
			})
			.OnMouseButtonDown_Lambda([this](const FGeometry&, const FPointerEvent&) {
				return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
			})
		]];
}

void SMGSequenceBlendHandle::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SetCursor(EMouseCursor::ResizeLeftRight);
	bIsHovered = true;
}

void SMGSequenceBlendHandle::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SetCursor(EMouseCursor::Default);
	bIsHovered = false;
}

FReply SMGSequenceBlendHandle::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && SectionDataPtr.IsValid() && ControllerPtr)
	{
		return FReply::Handled()
			.BeginDragDrop(FMGBlendHandleDragDrop::New(SectionDataPtr, ControllerPtr, HandleType))
			.ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

// ============================================================

/** Constructs this widget with InArgs */
void SMGSequenceTrackSection::Construct(const FArguments& InArgs)
{
	FDopeSheetStyle::SectionBodyBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Body");
	FDopeSheetStyle::SectionBodyBrush_Hovered = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Body.Hovered");
	FDopeSheetStyle::SectionBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Border");

	SequencePtr    = InArgs._Sequence;
	SectionDataPtr = InArgs._SectionData;
	ControllerPtr  = InArgs._Controller;

	SetCursor(EMouseCursor::GrabHand);
	ChildSlot[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor::Transparent)
		.Padding(TAttribute<FMargin>::CreateLambda([this]() -> FMargin {
			if (!SectionDataPtr.IsValid())
				return FMargin(0);
			const UMontageTrackSection_Sequences* Data  = SectionDataPtr.Get();
			const float                           Width = GetCachedGeometry().GetLocalSize().X;
			const float                           Dur   = static_cast<float>(Data->EndTime - Data->StartTime);
			if (Dur <= 0.f || Width <= 0.f)
				return FMargin(0);
			const float BlendInPx  = FMath::Min(Data->BlendInSettings.Blend.BlendTime * (Width / Dur), Width * 0.5f);
			const float BlendOutPx = FMath::Min(Data->BlendOutSettings.Blend.BlendTime * (Width / Dur), Width * 0.5f);
			return FMargin(FMath::Max(0.f, BlendInPx), 0.f, FMath::Max(0.f, BlendOutPx), 0.f);
		}))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.ColorAndOpacity(FDopeSheetStyle::AnimSequenceColor)
				.Image_Lambda([&]() {
					 return bIsHovered
					 		   ? FDopeSheetStyle::SectionBodyBrush_Hovered
					 		   : FDopeSheetStyle::SectionBodyBrush;
				})
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				.AutoWidth()
				[
					SNew(SMGSequenceBlendHandle)
					.SectionData(SectionDataPtr)
					.Controller(ControllerPtr)
					.HandleType(EMGBlendHandleType::BlendIn)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				.FillWidth(1.f)
				[

					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					.AutoWidth()
					.Padding(5)
					[
						SNew(SImage).Image(FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimSequence").GetIcon())
					]
					+ SHorizontalBox::Slot()
					.Padding(FMargin(2.f, 0.f, 0, 0))
					.AutoWidth()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(InArgs._Sequence->GetName()))
						.Justification(ETextJustify::Center)
					]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.AutoWidth()
				[
					SNew(SMGSequenceBlendHandle)
					.SectionData(SectionDataPtr)
					.Controller(ControllerPtr)
					.HandleType(EMGBlendHandleType::BlendOut)
				]
			]

		]
	];
}


int32 SMGSequenceTrackSection::OnPaint(
	const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 InLayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 LayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
	                                         InLayerId, InWidgetStyle, bParentEnabled);

	UMontageTrackSection_Sequences* SectionData = SectionDataPtr.Get();
	if (!SectionData) return LayerId;

	const float Width    = AllottedGeometry.GetLocalSize().X;
	const float Height   = AllottedGeometry.GetLocalSize().Y;
	const float Duration = static_cast<float>(SectionData->EndTime - SectionData->StartTime);
	if (Duration <= 0.f || Width <= 0.f) return LayerId;

	const float        PixelsPerSec = Width / Duration;
	const FSlateBrush* WhiteBrush   = FAppStyle::GetBrush("WhiteBrush");

	auto DrawBlendCurve = [&](float StartX, float BW, const FAlphaBlendArgs& Blend,
	                          bool bBlendIn, FLinearColor Color)
	{
		const EAlphaBlendOption BlendOpt = Blend.BlendOption;
		UCurveFloat*            Custom   = Blend.CustomCurve.Get();

		// // One seamless dark box covering the full blend region.
		// // Using per-strip dark boxes creates a visible horizontal staircase; a single box avoids it.
		// FSlateDrawElement::MakeBox(
		// 	OutDrawElements, LayerId + 1,
		// 	AllottedGeometry.ToPaintGeometry(FVector2D(StartX, 0.f), FVector2D(BW, Height)),
		// 	WhiteBrush, ESlateDrawEffect::None,
		// 	FLinearColor(0.f, 0.f, 0.f, 0.42f));

		// Fill strips — one per pixel so the staircase is sub-pixel and invisible.
		const int32 NumStrips = FMath::Max(1, FMath::CeilToInt(BW));
		const float StripW    = BW / static_cast<float>(NumStrips);
		for (int32 i = 0; i < NumStrips; ++i)
		{
			const float t      = (static_cast<float>(i) + 0.5f) / static_cast<float>(NumStrips);
			const float Alpha  = FAlphaBlend::AlphaToBlendOption(bBlendIn ? 1.f - t : t, BlendOpt, Custom);
			const float X      = StartX + static_cast<float>(i) * StripW;
			const float CurveY = Height * (1.f - Alpha);
			if (Height - CurveY > 0.5f)
			{
				FSlateDrawElement::MakeBox(
					OutDrawElements, LayerId + 1,
					AllottedGeometry.ToPaintGeometry(FVector2D(X, CurveY), FVector2D(StripW + 0.5f, Height - CurveY)),
					WhiteBrush, ESlateDrawEffect::None,
					Color.CopyWithNewOpacity(FMath::Pow(Alpha,1.25f)));
			}
		}

		////////////////////////////////////////////////////////////////////////
		// Bright line tracing the top of the blend curve
		////////////////////////////////////////////////////////////////////////
		const int32       NumPts = FMath::Max(2, NumStrips + 1);
		TArray<FVector2D> CurvePoints;
		CurvePoints.Reserve(NumPts);
		for (int32 i = 0; i < NumPts; ++i)
		{
			const float t     = static_cast<float>(i) / static_cast<float>(NumPts - 1);
			const float Alpha = FAlphaBlend::AlphaToBlendOption(bBlendIn ? 1.f - t : t, BlendOpt, Custom);
			CurvePoints.Add(FVector2D(StartX + t * BW, Height * (1.f - Alpha)));
		}
		FSlateDrawElement::MakeLines(
			OutDrawElements, LayerId + 2,
			AllottedGeometry.ToPaintGeometry(),
			CurvePoints,
			ESlateDrawEffect::None,
			FLinearColor(0.3f, 0.85f, 0.5f, .95f),
			true, 3.f);
	};

	// Blend-in — left side, green.
	const float BlendInTime = SectionData->BlendInSettings.Blend.BlendTime;
	if (BlendInTime > 0.f)
	{
		const float BW = FMath::Min(BlendInTime * PixelsPerSec, Width * 0.5f);
		DrawBlendCurve(0.f, BW, SectionData->BlendInSettings.Blend,
		               /*bInvert=*/false, FDopeSheetStyle::AnimSequenceColor);
	}

	// Blend-out — right side, orange. bInvert=true so weight reads 1→0 left-to-right.
	const float BlendOutTime = SectionData->BlendOutSettings.Blend.BlendTime;
	if (BlendOutTime > 0.f)
	{
		const float BW = FMath::Min(BlendOutTime * PixelsPerSec, Width * 0.5f);
		DrawBlendCurve(Width - BW, BW, SectionData->BlendOutSettings.Blend,
		               /*bInvert=*/true, FDopeSheetStyle::AnimSequenceColor );
	}

	return LayerId + 2;
}


void SMGSequenceTrackSection::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsHovered = true;
};

void SMGSequenceTrackSection::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	bIsHovered = false;
};

void SMGSequenceTrackSection::Select()
{
};

FReply SMGSequenceTrackSection::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && SequencePtr)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(
			TEXT("AssetTools"));
		TWeakPtr<IAssetTypeActions> AssetTypeActions = AssetToolsModule.Get().GetAssetTypeActionsForClass(
			UAnimSequence::StaticClass());
		if (AssetTypeActions.IsValid() && SequencePtr)
		{
			const TArray<UObject*> AssetsToOpen{SequencePtr};
			AssetTypeActions.Pin()->OpenAssetEditor(AssetsToOpen);
		}
	}
	return FReply::Handled();
};


FVector2D SMGSequenceTrackSection::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return GetCachedGeometry().Size;
}

void SMGSequenceTrack::Construct(const FArguments&                           InArgs,
                                 const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr)
{
	TrackModel = TrackModelPtr;

	if (TrackModel.IsValid() && TrackModel->ObjPtr)
	{
		TrackModel->ObjPtr->OnTrackPropertiesChanged.AddRaw(
			this, &SMGSequenceTrack::RegenerateSequenceSections);
	}


	RegenerateSequenceSections();
}

SMGSequenceTrack::~SMGSequenceTrack()
{
	if (TrackModel.IsValid() && TrackModel->ObjPtr)
	{
		TrackModel->ObjPtr->OnTrackPropertiesChanged.RemoveAll(this);
	}
}


FReply SMGSequenceTrack::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FVector2D ScreenCursorPos = MouseEvent.GetScreenSpacePosition();
		FVector2D SectionOffset   = FVector2D(SelectedSectionWidget->GetCachedGeometry().GetAbsolutePosition());

		bPreviewSectionRearrange = true;

		return FReply::Handled().BeginDragDrop(
			FDopeSheetSequenceSectionDragDrop::New(SharedThis(this), SelectedSectionWidget, ScreenCursorPos,
			                                       SectionOffset)).ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

void SMGSequenceTrack::PreviewRearrange(FVector2D ScreenCursorPos)
{
	RearrangeIndices.Reset();
	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		RearrangeIndices.Add(ChildIndex);
	}

	//Find Section Being Dragged Over
	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		const TSharedRef<SWidget> SectionWidget = TrackSectionWidgets[ChildIndex];

		const EVisibility ChildVisibility = SectionWidget->GetVisibility();
		if (ChildVisibility != EVisibility::Visible) { continue; }

		FSlateRect SectionRect = SectionWidget->GetCachedGeometry().GetRenderBoundingRect();

		if (SectionRect.Left < ScreenCursorPos.X
			&& SectionRect.Right > ScreenCursorPos.X
			&& SelectedSectionIndex != ChildIndex)
		{
			RearrangeIndices.Swap(ChildIndex, SelectedSectionIndex);
			break;
		}
	}
}

FReply SMGSequenceTrack::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();


	if (Operation->IsOfType<FDopeSheetSequenceSectionDragDrop>() && bPreviewSectionRearrange)
	{
	}


	if (Operation->IsOfType<FAssetDragDropOp>())
	{
		if ((TrackModel != nullptr) && (TrackModel->ObjPtr != nullptr))
		{
			TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(Operation);
			FText                        TooltipText;
			if (AssetOp->HasAssets())
			{
				const TArray<FAssetData>& HoveredAssetData = AssetOp->GetAssets();
				FText                     AssetReferenceFilterFailureReason;
				if (PassesAssetReferenceFilter(HoveredAssetData))
				{
					FString TooltipTextString;
					// TrackModel->GetAssetsHoverMessage(HoveredAssetData, TrackModel, /*out*/
					//                                                   TooltipTextString, /*out*/ bOkIcon);
					TooltipText       = FText::FromString(TooltipTextString);
					bIsValidAssetDrop = true;
				}
				else
				{
					TooltipText       = AssetReferenceFilterFailureReason;
					bIsValidAssetDrop = false;
				}
			}
			const FSlateBrush* TooltipIcon = bIsValidAssetDrop
				                                 ? FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK"))
				                                 : FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
			AssetOp->SetToolTip(TooltipText, TooltipIcon);
		}
		return FReply::Handled();
	}
	else
	{
		return SDopeSheetTrackTimeline::OnDragOver(MyGeometry, DragDropEvent);
	}
}

bool SMGSequenceTrack::PassesAssetReferenceFilter(const TArray<FAssetData>& Array)
{
	bool bIsValid = true;
	for (auto Asset : Array)
	{
		if (!Asset.IsInstanceOf(UAnimSequence::StaticClass()))
		{
			bIsValid = false;
			break;
		}
	}

	return bIsValid;
}


void SMGSequenceTrack::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	bIsValidAssetDrop = false;
}

void SMGSequenceTrack::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	bIsValidAssetDrop = false;
}

FReply SMGSequenceTrack::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if ((TrackModel != nullptr) && (TrackModel->ObjPtr != nullptr))
	{
		TArray<FAssetData> DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(DragDropEvent);

		if (DroppedAssetData.Num() > 0)
		{
			if (PassesAssetReferenceFilter(DroppedAssetData))
			{
				TrackModel->ObjPtr->DroppedAssetsOnTrack(DroppedAssetData);
			}
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

int32 SMGSequenceTrack::OnPaint(const FPaintArgs&   Args, const FGeometry&                  AllottedGeometry,
                                const FSlateRect&   MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                int32               InLayerId,
                                const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	static const FSlateBrush* PreviewBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Preview");
	static const FSlateBrush* SectionBorderBrush = FMontageGraphEditorStyle::Get().GetBrush(
		"MontageGraph.SequenceTrack.Section.Border");


	if (bPreviewSectionRearrange)
	{
		const float AllottedWidth     = static_cast<float>(AllottedGeometry.GetLocalSize().X);
		const float AllottedHeight    = static_cast<float>(AllottedGeometry.GetLocalSize().Y);
		float       AccumulatedOffset = TrackModel->Controller->EditableRect.Left;
		for (int32 i = 0; i < RearrangeIndices.Num(); ++i)
		{
			int32 Index   = RearrangeIndices[i];
			auto  Section = TrackModel->SectionModels[Index];

			const float ViewedTimeCoefficient = Section->GetTimeLength() / TrackModel->Controller->ViewTimeLength;
			const float SectionWidth          = AllottedWidth * ViewedTimeCoefficient;

			FSlateDrawElement::MakeBox
			(
				OutDrawElements,
				InLayerId + 1,
				AllottedGeometry.ToPaintGeometry(FVector2D(AccumulatedOffset, 0),
				                                 FVector2D(SectionWidth, AllottedHeight)),
				Index == SelectedSectionIndex ? PreviewBorderBrush : SectionBorderBrush,
				ESlateDrawEffect::None,
				Index == SelectedSectionIndex ? FColor(168, 255, 153, 255) : FColor(110, 110, 110, 255)
			);


			AccumulatedOffset += SectionWidth;
		}


		return InLayerId;
	}


	int32 LayerId = SDopeSheetTrackTimeline::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, InLayerId,
	                                                 InWidgetStyle, bParentEnabled);


	if (bIsValidAssetDrop)
	{
		FSlateDrawElement::MakeBox
		(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			SectionBorderBrush,
			ESlateDrawEffect::None,
			FColor(80, 123, 72, 255)
		);
	}

	return LayerId;
}

void SMGSequenceTrack::OnRearrangeDrop()
{
	if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(TrackModel->ObjPtr))
	{
		TArray<UDopeSheetTrackSection> Temp;

		//
		// for (int32 i = 0; i < RearrangeIndices.Num(); ++i)
		// {
		// 	int32 Index = RearrangeIndices[i];
		//
		// 	if (UMontageTrackSection_Sequences* MontagePart = Cast<UMontageTrackSection_Sequences>(
		// 		SequencesTrack->Sections[i]))
		// 	{
		// 		Temp.Add(SequencesTrack->Sections[Index]);
		// 	}
		// }

	}
	bPreviewSectionRearrange = false;
	TrackModel->ObjPtr->OnTrackPropertiesChanged.Broadcast();
}



void SMGSequenceTrack::OnArrangeChildren(const FGeometry&   AllottedGeometry,
                                                FArrangedChildren& ArrangedChildren) const
{
	if (!TrackModel)
	{
		return;
	}

	const float AllottedWidth  = static_cast<float>(AllottedGeometry.GetLocalSize().X);
	const float TimeOffsetPx   = TrackModel->Controller->ViewTimeLength > 0.f
		? static_cast<float>(TrackModel->DisplayTimeOffset / TrackModel->Controller->ViewTimeLength) * AllottedWidth
		: 0.f;

	float AccumulatedOffset = TrackModel->Controller->EditableRect.Left + TimeOffsetPx;
	for (int32 ChildIndex = 0; ChildIndex < TrackSectionWidgets.Num(); ++ChildIndex)
	{
		const TSharedRef<SWidget> ChildWidget = TrackSectionWidgets[ChildIndex];

		const EVisibility ChildVisibility = ChildWidget->GetVisibility();
		if (!ArrangedChildren.Accepts(ChildVisibility))
		{
			continue;
		}

		const float AllottedHeight = static_cast<float>(AllottedGeometry.GetLocalSize().Y);
			
		auto Section = TrackModel->SectionModels[ChildIndex];
		
		const float ViewedTimeCoefficient = Section->GetTimeLength() / TrackModel->Controller->ViewTimeLength;
		const float SectionWidth = AllottedWidth * ViewedTimeCoefficient;
		

		ArrangedChildren.AddWidget( AllottedGeometry.MakeChild(
			                           ChildWidget,
			                           FVector2D(AccumulatedOffset, 0),
			                           FVector2D(SectionWidth, AllottedHeight)));
		
		
		AccumulatedOffset += SectionWidth;
	}
}

void SMGSequenceTrack::RegenerateSequenceSections()
{
	TrackSectionWidgets.Reset();


	if ((TrackModel != nullptr) && (TrackModel->ObjPtr))
	{
		if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(TrackModel->ObjPtr))
		{
			const int NumOfSections = SequencesTrack->Sections.Num();
			TrackModel->SectionModels.Reset(NumOfSections);

			float AccumulatedTime = 0.f;
			for (int i = 0; i < NumOfSections; i++)
			{
				if (UMontageTrackSection_Sequences* MontageSection =
					Cast<UMontageTrackSection_Sequences>(SequencesTrack->Sections[i]))
				{
					
					UAnimSequence* AnimSequence = MontageSection->AnimSequence;

					const float StartTime = AccumulatedTime;
					AccumulatedTime += AnimSequence->GetPlayLength();

					MontageSection->StartTime = StartTime;
					MontageSection->EndTime   = AccumulatedTime;
					
					TrackModel->SectionModels.Add(MakeShared<FDopeSheetSectionViewModel>(MontageSection, TrackModel.Get()));

					TrackSectionWidgets.Add(
					SNew(SMGSequenceTrackSection)
					.Sequence(AnimSequence)
					.SectionData(MontageSection)
					.Controller(TrackModel->Controller));
				}
			}
		}
	}
}


void UMontageTrack_Sequences::BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
                                          UMontageGraph* OwnerGraph, const FString& DisplayName)
{
	// Bake the AnimMontage asset from this track's sequence sections.
	BakedData.Montage = CreateNewDataObject<UAnimMontage>(OwnerGraph, FName(DisplayName + "_Montage"));

	// Scrape blend links from sub-tracks and write them directly to the runtime node.
	RuntimeNode->BlendLinks.Empty();
	for (UDopeSheetTrackBase* SubTrack : SubTracks)
	{
		if (UMontageTrack_BlendLinks* LinkTrack = Cast<UMontageTrack_BlendLinks>(SubTrack))
		{
			for (UDopeSheetTrackSection* Section : LinkTrack->Sections)
			{
				if (UMontageTrackSection_LinkBlend* LinkSection = Cast<UMontageTrackSection_LinkBlend>(Section))
				{
					if (!LinkSection->TargetNode) { continue; }
					if (UMGNode_Montage* LinkToNode = Cast<UMGNode_Montage>(LinkSection->TargetNode->GetRuntimeNode()))
					{
						FMontageGraphLinkSettings& LinkRef = RuntimeNode->BlendLinks.Add(LinkToNode, LinkSection->LinkSettings);
						LinkRef.StartTime = LinkSection->StartTime;
					}
				}
			}
		}

		// Apply named montage sections — overrides the per-clip sections written by GenerateNewDataAsset
		if (UMontageTrack_Sections* SectionsTrack = Cast<UMontageTrack_Sections>(SubTrack))
		{
			SectionsTrack->ApplyToMontage(BakedData.Montage);
		}
	}
}

UMontageTrack_Sequences::UMontageTrack_Sequences(const FObjectInitializer& ObjectInitializer)
{
	UMontageTrack_BlendLinks* BlendLinksTrack =
		ObjectInitializer.CreateDefaultSubobject<UMontageTrack_BlendLinks>(this,TEXT("MontageTrack_BlendLinks_DefaultBlend"));
	BlendLinksTrack->bAllowRename = false;

	SubTracks.Add(BlendLinksTrack);

	bAllowRename  = false;
	bAllowDelete  = false;  // Sequences track is mandatory — it owns the montage asset.
	bShouldDrawCells = false;
	CollectionName = FName("Sequences");
}

UObject* UMontageTrack_Sequences::GenerateNewDataAsset(UObject* Outer, FName Name)
{
	// UMGEdNode_Montage* MontageEdNode = Cast<UMGEdNode_Montage>(NodeBase);
	// UMGNode_Montage* MontageNode = Cast<UMGNode_Montage>(MontageEdNode->RuntimeNode);
	//
	// FName MontageName = FName(OwningGraph->GetName() + '_' + MontageEdNode->
	// 	                                                   MontageDisplayName);

	UAnimMontage* NewMontage = NewObject<UAnimMontage>(Outer, Name, RF_Public | RF_Standalone);


	if (Sections.IsEmpty())
	{
		return NewMontage;
	}

	if (UMontageTrackSection_Sequences* MontageSection = Cast<UMontageTrackSection_Sequences>(Sections[0]))
	{
		UAnimSequence* AnimAsset = MontageSection->AnimSequence;
		if (!AnimAsset)
		{
			return nullptr;
		}
		USkeleton* AssetSkeleton = AnimAsset->GetSkeleton();
		if (!AssetSkeleton)
		{
			return nullptr;
		}


		NewMontage->SetSkeleton(AssetSkeleton);
	}

	// Build track structure
	TMap<FName, int32> SlotToTrackIndex;
	TArray<float>      TrackLengths;

	// Collect unique slot names and create tracks
	TSet<FName> UniqueSlotNames;
	for (const auto Section : Sections)
	{
		if (UMontageTrackSection_Sequences* MontageSection = Cast<UMontageTrackSection_Sequences>(Section))
		{
			UniqueSlotNames.Add(MontageSection->SlotName);
		}
	}

	NewMontage->SlotAnimTracks.Reset();
	for (const FName& SlotName : UniqueSlotNames)
	{
		int32                TrackIndex = NewMontage->SlotAnimTracks.Num();
		FSlotAnimationTrack& NewTrack   = NewMontage->AddSlot(SlotName);
		NewTrack.SlotName               = SlotName;
		SlotToTrackIndex.Add(SlotName, TrackIndex);
		TrackLengths.Add(0.0f);
	}

	// Process segments and build animation data
	float              TotalMontageLength = 0.0f;
	TMap<FName, float> SlotCurrentTime;

	for (const auto Section : Sections)
	{
		if (UMontageTrackSection_Sequences* MontageSection = Cast<UMontageTrackSection_Sequences>(Section))
		{
			UAnimSequence* Anim          = MontageSection->AnimSequence;
			float          SegmentLength = Anim->GetPlayLength();


			// Create animation segment
			FAnimSegment NewSegment;
			NewSegment.SetAnimReference(Anim, true);
			NewSegment.LoopingCount = 1;
			NewSegment.StartPos     = SlotCurrentTime.FindOrAdd(MontageSection->SlotName, 0.0f);

			int32 TrackIndex = SlotToTrackIndex[MontageSection->SlotName];
			NewMontage->SlotAnimTracks[TrackIndex].AnimTrack.AnimSegments.Add(NewSegment);

			// Create composite section
			FCompositeSection NewSection;
			NewSection.SectionName = MontageSection->SectionName;
			NewSection.Link(Anim, SegmentLength);
			NewSection.SetTime(SlotCurrentTime[MontageSection->SlotName]);
			NewMontage->CompositeSections.Add(NewSection);

			// Update timing tracking
			SlotCurrentTime[MontageSection->SlotName] += SegmentLength;
			TrackLengths[TrackIndex] = FMath::Max(TrackLengths[TrackIndex], SlotCurrentTime[MontageSection->SlotName]);
		}
	}
	// Calculate total montage length
	for (float TrackLength : TrackLengths)
	{
		TotalMontageLength = FMath::Max(TotalMontageLength, TrackLength);
	}
	//    
	//    // Apply blend settings
	// NewMontage->BlendIn = FAlphaBlend(MontageEdNode->BlendInSettings.Blend);
	// NewMontage->BlendModeIn = MontageEdNode->BlendInSettings.BlendMode;
	// NewMontage->BlendProfileIn = MontageEdNode->BlendInSettings.BlendProfile;
	//
	// NewMontage->BlendOut = FAlphaBlend(MontageEdNode->BlendOutSettings.Blend);
	// NewMontage->BlendModeOut = MontageEdNode->BlendOutSettings.BlendMode;
	// NewMontage->BlendProfileOut = MontageEdNode->BlendOutSettings.BlendProfile;
	//
	// NewMontage->BlendOutTriggerTime = MontageEdNode->InBlendOutTriggerTime;
	//    
	// Finalize montage
	NewMontage->SetCompositeLength(TotalMontageLength);


	// Trigger post-edit to ensure proper setup
	FProperty*            ChangedProperty = nullptr;
	FPropertyChangedEvent PropertyEvent(ChangedProperty);
	NewMontage->PostEditChangeProperty(PropertyEvent);

	MG_LOG("Created montage %s with %d segments and length %.2f",
	       *NewMontage->GetName(), Sections.Num(), TotalMontageLength);
	return NewMontage;
}

void SMGSequenceTrack::MakeSectionContextMenu(FMenuBuilder& ContextMenuBuilder)
{
	static constexpr float  ThumbnailIconSize       = 64.0f;
	static constexpr uint32 ThumbnailIconResolution = 64;
	
	if (!(TrackModel) || !(TrackModel->ObjPtr))
	{
		return;
	}

	if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(TrackModel->ObjPtr))
	{
		if (!SequencesTrack->Sections.IsValidIndex(SelectedSectionIndex))
		{
			return;
		}
		UMontageTrackSection_Sequences* MontageSection =
			Cast<UMontageTrackSection_Sequences>(
				SequencesTrack->Sections[SelectedSectionIndex]);
		if (!MontageSection)
		{
			return;
		}

		UAnimSequence* AnimSequence = MontageSection->AnimSequence;
		if (!AnimSequence) { return; }

		ContextMenuBuilder.BeginSection(TEXT("MontageSection"), LOCTEXT("MontageSection", "Montage Section"));
		{
			FAssetThumbnailConfig ThumbnailConfig;
			//
			// if (FSlateApplication::Get().InKismetDebuggingMode())
			// {
			// 	ThumbnailConfig.bForceGenericThumbnail = true;
			// }


			TSharedPtr<FAssetThumbnail> Thumb = MakeShared<FAssetThumbnail>(AnimSequence,
			                                                                ThumbnailIconResolution,
			                                                                ThumbnailIconResolution,
			                                                                UThumbnailManager::Get().
			                                                                GetSharedThumbnailPool());
			auto AnimSectionDetails = SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("NoBorder"))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.AutoWidth()
					[
						SNew(SBox)
						.MaxDesiredHeight(ThumbnailIconSize)
						.MaxDesiredWidth(ThumbnailIconSize)
						[
							Thumb->MakeThumbnailWidget(ThumbnailConfig)
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Center)
					.Padding(FMargin(5, 0))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(AnimSequence->GetName()))
						]
						+ SVerticalBox::Slot()
						.Padding(FMargin(0, 5))
						.AutoHeight()
						.HAlign(HAlign_Fill)
						[
							SNew(SAnimMontageSlotPicker, AnimSequence, MontageSection->SlotName)
							.OnSlotChanged(FOnSlotChanged::CreateLambda([this](const FName NewSlotName)
							{
								if (UMontageTrack_Sequences* SequencesTrack = Cast<UMontageTrack_Sequences>(
									TrackModel->ObjPtr))
								{
									if (UMontageTrackSection_Sequences* MontageSections =
										Cast<UMontageTrackSection_Sequences>(
											SequencesTrack->Sections[SelectedSectionIndex]))
									{
										MontageSections->SlotName = NewSlotName;
										TrackModel->ObjPtr->OnTrackPropertiesChanged.Broadcast();
									}
								}
							}))
						]
					]
				];


			ContextMenuBuilder.AddWidget(AnimSectionDetails, FText::GetEmpty(), true);
			FUIAction ItemAction(FExecuteAction::CreateLambda([this]()
			{
				TrackModel->DeleteSection(SelectedSectionIndex);
			}));
			ContextMenuBuilder.AddMenuEntry(FText::FromString("Delete Section"), TAttribute<FText>(),
			                                FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage"),
			                                ItemAction);
		}
		ContextMenuBuilder.EndSection();
	}
}

void UMontageTrack_Sequences::DroppedAssetsOnTrack(TArray<FAssetData> Array)
{
	bool bBroadcastChanges = false;
	for (auto Asset : Array)
	{
		if (Asset.IsInstanceOf(UAnimSequence::StaticClass()))
		{
			if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(Asset.GetAsset()))
			{
				UMontageTrackSection_Sequences* NewMontageSection = NewObject<UMontageTrackSection_Sequences>(this);
				NewMontageSection->AnimSequence = AnimSequence;

				Sections.Add(NewMontageSection);

				bBroadcastChanges = true;
			}
		}
	}
	if (bBroadcastChanges)
	{
		OnTrackPropertiesChanged.Broadcast();
	}
}



bool UMontageTrack_Sequences::CanCreateSubTracks()
{
	return true;
}

void UMontageTrack_Sequences::GetSubTrackClasses(TArray<UClass*>& TrackClasses)
{
	TrackClasses.Add(UMontageTrack_BlendLinks::StaticClass());
	TrackClasses.Add(UMontageTrack_Sections::StaticClass());
}

TSharedRef<SWidget> UMontageTrack_Sequences::MakeTrackTimelineWidget(
	TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
{
	return SNew(SMGSequenceTrack, TrackModelPtr);
}

const FText UMontageTrack_Sequences::GetTrackName() const
{
	return FText::FromString("Sequences");
}

const FLinearColor UMontageTrack_Sequences::GetTrackColor() const
{
	return FColor(80, 123, 72, 255);
}

FSlateIcon UMontageTrack_Sequences::GetTrackIcon() const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimSequence");
}
#undef LOCTEXT_NAMESPACE
#endif
