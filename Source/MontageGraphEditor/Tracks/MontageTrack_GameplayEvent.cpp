#include "MontageTrack_GameplayEvent.h"

#include "MontageGraphEditorStyle.h"
#include "ScopedTransaction.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/Tracks/DopeSheetTrackViewModel.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "MontageGraph/Notifies/MG_AnimNotify_GameplayEvent.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Animation/AnimMontage.h"

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "MontageTrack_GameplayEvent"

// ============================================================
// FMGKeyframeDragDropOp
// ============================================================

TSharedRef<FMGKeyframeDragDropOp> FMGKeyframeDragDropOp::New(
	TWeakObjectPtr<UMontageTrackSection_GameplayEvent> InSection,
	TSharedPtr<FDopeSheetController>                  InController,
	const FVector2D&                                  InStartPos)
{
	TSharedRef<FMGKeyframeDragDropOp> Op = MakeShareable(new FMGKeyframeDragDropOp);
	Op->SectionPtr    = InSection;
	Op->ControllerPtr = InController;
	Op->StartScreenPos = InStartPos;
	if (UMontageTrackSection_GameplayEvent* S = InSection.Get())
	{
		S->Modify();
		Op->InitialTime = S->StartTime;
	}
	Op->Construct();
	return Op;
}

void FMGKeyframeDragDropOp::Construct()
{
	FDragDropOperation::Construct();
}

TSharedPtr<SWidget> FMGKeyframeDragDropOp::GetDefaultDecorator() const
{
	UMontageTrackSection_GameplayEvent* S = SectionPtr.Get();
	const FString Label = S && S->EventTag.IsValid()
		? FString::Printf(TEXT("Event: %s"), *S->EventTag.GetTagName().ToString())
		: TEXT("Gameplay Event");

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		[SNew(STextBlock).Text(FText::FromString(Label))];
}

void FMGKeyframeDragDropOp::OnDragged(const FDragDropEvent& DragDropEvent)
{
	UMontageTrackSection_GameplayEvent* S = SectionPtr.Get();
	if (!S || !ControllerPtr) { return; }

	const FVector2D MousePos = DragDropEvent.GetScreenSpacePosition();
	const double    Delta    = ControllerPtr->AbsoluteXCoordToTime(MousePos.X)
	                         - ControllerPtr->AbsoluteXCoordToTime(StartScreenPos.X);

	S->StartTime = FMath::Max(0.0, InitialTime + Delta);
	S->EndTime   = S->StartTime;

	CursorDecoratorWindow->MoveWindowTo(MousePos + FVector2D(12.f, -12.f));
}

void FMGKeyframeDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	if (UMontageTrackSection_GameplayEvent* S = SectionPtr.Get())
	{
		if (UMontageTrack_GameplayEvent* Track = S->GetTypedOuter<UMontageTrack_GameplayEvent>())
		{
			Track->OnTrackPropertiesChanged.Broadcast();
		}
	}
	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}

// ============================================================
// SMGGameplayEventTrack
// ============================================================

void SMGGameplayEventTrack::Construct(const FArguments& InArgs,
                                      const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr)
{
	// Base Construct hooks up OnTrackPropertiesChanged → RegenerateSections.
	// GameplayEvent track has no child section widgets (diamonds are painted directly),
	// so RegenerateSections will just clear TrackSectionWidgets — that's fine.
	SDopeSheetTrackTimeline::Construct(SDopeSheetTrackTimeline::FArguments(), TrackModelPtr);
}

// ----------------------------------------------------------------
// Hit-test — returns keyframe index under LocalPos, or -1
// ----------------------------------------------------------------

int32 SMGGameplayEventTrack::HitTestKeyframe(const FGeometry& Geometry, const FVector2D& LocalPos) const
{
	constexpr float HitRadius = 10.f;

	UMontageTrack_GameplayEvent* Track = Cast<UMontageTrack_GameplayEvent>(
		TrackModel ? TrackModel->ObjPtr : nullptr);
	if (!Track || !TrackModel->Controller) { return -1; }

	for (int32 i = 0; i < Track->Sections.Num(); i++)
	{
		const UMontageTrackSection_GameplayEvent* S = Cast<UMontageTrackSection_GameplayEvent>(Track->Sections[i]);
		if (!S) { continue; }

		const float X = static_cast<float>(TrackModel->Controller->TimeToXOffset(S->StartTime, Geometry));
		if (FMath::Abs(LocalPos.X - X) <= HitRadius)
		{
			return i;
		}
	}
	return -1;
}

// ----------------------------------------------------------------
// Mouse handling
// ----------------------------------------------------------------

FReply SMGGameplayEventTrack::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!TrackModel || !TrackModel->Controller) { return FReply::Unhandled(); }

	const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const int32     HitIdx   = HitTestKeyframe(MyGeometry, LocalPos);

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && HitIdx >= 0)
	{
		DragKeyframeIndex = HitIdx;
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		ContextKeyframeIndex = HitIdx;
		LastRightClickTime   = TrackModel->Controller->AbsoluteXCoordToTime(
			MouseEvent.GetScreenSpacePosition().X);
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	return FReply::Unhandled();
}

FReply SMGGameplayEventTrack::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && HasMouseCapture())
	{
		FMenuBuilder MenuBuilder(true, nullptr);
		if (ContextKeyframeIndex >= 0)
		{
			MakeSectionContextMenu(MenuBuilder);
		}
		else
		{
			MakeTrackContextMenu(MenuBuilder);
		}

		FSlateApplication::Get().PushMenu(
			FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef(),
			FWidgetPath(),
			MenuBuilder.MakeWidget(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect::ContextMenu);

		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled().ReleaseMouseCapture();
}

FReply SMGGameplayEventTrack::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && DragKeyframeIndex >= 0)
	{
		UMontageTrack_GameplayEvent* Track = Cast<UMontageTrack_GameplayEvent>(
			TrackModel ? TrackModel->ObjPtr : nullptr);
		if (Track && Track->Sections.IsValidIndex(DragKeyframeIndex))
		{
			if (UMontageTrackSection_GameplayEvent* S = Cast<UMontageTrackSection_GameplayEvent>(
				Track->Sections[DragKeyframeIndex]))
			{
				return FReply::Handled()
					.BeginDragDrop(FMGKeyframeDragDropOp::New(
						S, TrackModel->Controller,
						MouseEvent.GetScreenSpacePosition()))
					.ReleaseMouseCapture();
			}
		}
	}
	return FReply::Unhandled();
}

// ----------------------------------------------------------------
// Context menus
// ----------------------------------------------------------------

void SMGGameplayEventTrack::MakeTrackContextMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AddEvent", LOCTEXT("AddEventHeader", "Add Event"));
	{
		TSharedRef<SEditableTextBox> TagBox =
			SNew(SEditableTextBox)
			.HintText(LOCTEXT("TagHint", "gameplay.tag.name"))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type CommitType)
			{
				if (CommitType != ETextCommit::OnEnter || Text.IsEmpty()) { return; }

				const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(
					FName(*Text.ToString()), /*bErrorIfNotFound=*/false);
				if (!Tag.IsValid()) { return; }

				UMontageTrack_GameplayEvent* Track = Cast<UMontageTrack_GameplayEvent>(
					TrackModel ? TrackModel->ObjPtr : nullptr);
				if (!Track) { return; }

				const FScopedTransaction Transaction(LOCTEXT("AddEventTx", "Add Gameplay Event"));
				Track->Modify();

				UMontageTrackSection_GameplayEvent* NewSection =
					NewObject<UMontageTrackSection_GameplayEvent>(Track);
				NewSection->EventTag  = Tag;
				NewSection->StartTime = LastRightClickTime;
				NewSection->EndTime   = LastRightClickTime;
				Track->Sections.Add(NewSection);
				Track->OnTrackPropertiesChanged.Broadcast();
				FSlateApplication::Get().DismissAllMenus();
			});

		MenuBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(4.f, 2.f).FillWidth(1.f)[TagBox],
			FText::GetEmpty(), false);
	}
	MenuBuilder.EndSection();
}

void SMGGameplayEventTrack::MakeSectionContextMenu(FMenuBuilder& MenuBuilder)
{
	UMontageTrack_GameplayEvent* Track = Cast<UMontageTrack_GameplayEvent>(
		TrackModel ? TrackModel->ObjPtr : nullptr);
	if (!Track || !Track->Sections.IsValidIndex(ContextKeyframeIndex)) { return; }

	UMontageTrackSection_GameplayEvent* S = Cast<UMontageTrackSection_GameplayEvent>(
		Track->Sections[ContextKeyframeIndex]);
	if (!S) { return; }

	MenuBuilder.BeginSection("EventOps", FText::FromName(S->EventTag.GetTagName()));
	{
		// Inline tag rename
		TSharedRef<SEditableTextBox> TagBox =
			SNew(SEditableTextBox)
			.Text(FText::FromName(S->EventTag.GetTagName()))
			.OnTextCommitted_Lambda([this, Track, S](const FText& Text, ETextCommit::Type CommitType)
			{
				if (CommitType != ETextCommit::OnEnter || Text.IsEmpty()) { return; }
				const FGameplayTag NewTag = FGameplayTag::RequestGameplayTag(
					FName(*Text.ToString()), false);
				if (!NewTag.IsValid()) { return; }

				const FScopedTransaction Tx(LOCTEXT("RenameEventTagTx", "Change Event Tag"));
				S->Modify();
				S->EventTag = NewTag;
				Track->OnTrackPropertiesChanged.Broadcast();
				FSlateApplication::Get().DismissAllMenus();
			});

		MenuBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(4.f, 2.f).FillWidth(1.f)[TagBox],
			LOCTEXT("TagLabel", "Tag"));

		// Delete
		const int32 CapturedIdx = ContextKeyframeIndex;
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteEvent", "Delete Event"),
			FText::GetEmpty(),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
			FUIAction(FExecuteAction::CreateLambda([this, Track, CapturedIdx]()
			{
				const FScopedTransaction Tx(LOCTEXT("DeleteEventTx", "Delete Gameplay Event"));
				Track->Modify();
				Track->Sections.RemoveAt(CapturedIdx);
				Track->OnTrackPropertiesChanged.Broadcast();
			})));
	}
	MenuBuilder.EndSection();
}

// ----------------------------------------------------------------
// Painting — diamond icon at each keyframe time
// ----------------------------------------------------------------

int32 SMGGameplayEventTrack::OnPaint(
	const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 InLayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 LayerId = SDopeSheetTrackTimeline::OnPaint(Args, AllottedGeometry, MyCullingRect,
	                                                  OutDrawElements, InLayerId, InWidgetStyle, bParentEnabled);

	UMontageTrack_GameplayEvent* Track = Cast<UMontageTrack_GameplayEvent>(
		TrackModel ? TrackModel->ObjPtr : nullptr);
	if (!Track || !TrackModel->Controller) { return LayerId; }

	const float Height = AllottedGeometry.GetLocalSize().Y;
	const float CY     = Height * 0.5f;
	constexpr float S  = 8.f;   // diamond half-extent

	static const FLinearColor DiamondColor(1.0f, 0.85f, 0.0f, 1.0f);   // gold-yellow
	static const FLinearColor DiamondHighlight(1.f, 1.f, 0.6f, 1.f);

	const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Bold", 7);

	for (int32 i = 0; i < Track->Sections.Num(); i++)
	{
		const UMontageTrackSection_GameplayEvent* Kf = Cast<UMontageTrackSection_GameplayEvent>(Track->Sections[i]);
		if (!Kf) { continue; }

		const float X = static_cast<float>(TrackModel->Controller->TimeToXOffset(Kf->StartTime, AllottedGeometry));

		// Vertical tick line
		const TArray<FVector2D> TickLine = { FVector2D(X, 0.f), FVector2D(X, Height) };
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			TickLine, ESlateDrawEffect::None,
			DiamondColor.CopyWithNewOpacity(0.4f), true, 1.f);

		// Diamond outline (5-point closed polygon)
		const TArray<FVector2D> Diamond = {
			FVector2D(X,     CY - S),  // top
			FVector2D(X + S, CY),      // right
			FVector2D(X,     CY + S),  // bottom
			FVector2D(X - S, CY),      // left
			FVector2D(X,     CY - S),  // close
		};
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2,
			AllottedGeometry.ToPaintGeometry(),
			Diamond, ESlateDrawEffect::None, DiamondColor, true, 2.f);

		// Inner fill (smaller diamond, slightly lighter)
		constexpr float InnerS = S - 3.f;
		const TArray<FVector2D> InnerDiamond = {
			FVector2D(X,          CY - InnerS),
			FVector2D(X + InnerS, CY),
			FVector2D(X,          CY + InnerS),
			FVector2D(X - InnerS, CY),
			FVector2D(X,          CY - InnerS),
		};
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2,
			AllottedGeometry.ToPaintGeometry(),
			InnerDiamond, ESlateDrawEffect::None,
			DiamondHighlight.CopyWithNewOpacity(0.6f), true, 1.f);

		// Tag name below the diamond
		if (Kf->EventTag.IsValid())
		{
			const FText Label = FText::FromName(Kf->EventTag.GetTagName());
			FSlateDrawElement::MakeText(OutDrawElements, LayerId + 3,
				AllottedGeometry.ToPaintGeometry(FVector2D(X + S + 2.f, CY - 6.f), FVector2D(100.f, 12.f)),
				Label, Font, ESlateDrawEffect::None,
				FLinearColor(1.f, 1.f, 1.f, 0.8f));
		}
	}

	return LayerId + 3;
}

// ============================================================
// UMontageTrack_GameplayEvent
// ============================================================

UMontageTrack_GameplayEvent::UMontageTrack_GameplayEvent()
{
	CollectionName   = FName("Gameplay");
	bAllowRename     = false;
	bShouldDrawCells = false;
}

void UMontageTrack_GameplayEvent::BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
                                              UMontageGraph* OwnerGraph, const FString& DisplayName)
{
	if (!BakedData.Montage || Sections.IsEmpty()) { return; }

	for (UDopeSheetTrackSection* Raw : Sections)
	{
		const UMontageTrackSection_GameplayEvent* EvtSection =
			Cast<UMontageTrackSection_GameplayEvent>(Raw);
		if (!EvtSection || !EvtSection->EventTag.IsValid()) { continue; }

		UMG_AnimNotify_GameplayEvent* Notify = NewObject<UMG_AnimNotify_GameplayEvent>(BakedData.Montage);
		Notify->EventTag = EvtSection->EventTag;

		FAnimNotifyEvent NewNotify;
		NewNotify.NotifyName = EvtSection->EventTag.GetTagName();
		NewNotify.Notify     = Notify;
		NewNotify.TrackIndex = 1;
		NewNotify.Link(BakedData.Montage, static_cast<float>(EvtSection->StartTime));

		BakedData.Montage->Notifies.Add(NewNotify);
	}

	FProperty* Prop = nullptr;
	FPropertyChangedEvent Evt(Prop);
	BakedData.Montage->PostEditChangeProperty(Evt);
}

const FText UMontageTrack_GameplayEvent::GetTrackName() const
{
	return LOCTEXT("TrackName", "Gameplay Event");
}

const FLinearColor UMontageTrack_GameplayEvent::GetTrackColor() const
{
	return FLinearColor(1.0f, 0.85f, 0.0f, 1.f);
}

const FLinearColor UMontageTrack_GameplayEvent::GetSectionColor() const
{
	return FLinearColor(1.0f, 0.85f, 0.0f, 1.f);
}

FSlateIcon UMontageTrack_GameplayEvent::GetTrackIcon() const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.GameplayAbility");
}

TSharedRef<SWidget> UMontageTrack_GameplayEvent::MakeTrackTimelineWidget(
	TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
{
	return SNew(SMGGameplayEventTrack, TrackModelPtr);
}

#undef LOCTEXT_NAMESPACE
#endif
