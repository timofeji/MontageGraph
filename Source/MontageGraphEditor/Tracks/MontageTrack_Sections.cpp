#include "MontageTrack_Sections.h"

#include "MontageGraphEditorStyle.h"
#include "Animation/AnimMontage.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/Tracks/DopeSheetTrackViewModel.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SEditableTextBox.h"

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "MontageTrack_Sections"

// ============================================================
// UMontageTrack_Sections
// ============================================================

void UMontageTrack_Sections::ApplyToMontage(UAnimMontage* Montage) const
{
	if (!Montage) return;

	// Sort sections by start time
	TArray<UDopeSheetTrackSection*> Sorted = Sections;
	Sorted.Sort([](const UDopeSheetTrackSection& A, const UDopeSheetTrackSection& B)
	{
		return A.StartTime < B.StartTime;
	});

	Montage->CompositeSections.Reset();

	for (UDopeSheetTrackSection* Raw : Sorted)
	{
		UMontageTrackSection_SectionName* Marker = Cast<UMontageTrackSection_SectionName>(Raw);
		if (!Marker) continue;

		FCompositeSection CompositeSection;
		CompositeSection.SectionName = Marker->SectionName;
		CompositeSection.SetTime(static_cast<float>(Marker->StartTime));
		Montage->CompositeSections.Add(CompositeSection);
	}

	// Ensure a "Default" section always exists at the start of the montage
	const bool bHasDefault = Montage->CompositeSections.ContainsByPredicate(
		[](const FCompositeSection& S) { return S.SectionName == FName("Default"); });
	if (!bHasDefault)
	{
		FCompositeSection DefaultSection;
		DefaultSection.SectionName = FName("Default");
		DefaultSection.SetTime(0.f);
		Montage->CompositeSections.Insert(DefaultSection, 0);
	}

	FProperty* Prop = nullptr;
	FPropertyChangedEvent PropertyEvent(Prop);
	Montage->PostEditChangeProperty(PropertyEvent);
}

const FText UMontageTrack_Sections::GetTrackName() const
{
	return LOCTEXT("TrackName", "Sections");
}

const FLinearColor UMontageTrack_Sections::GetTrackColor() const
{
	return FLinearColor(0.7f, 0.55f, 1.0f, 1.f);
}

const FLinearColor UMontageTrack_Sections::GetSectionColor() const
{
	return FLinearColor(0.7f, 0.55f, 1.0f, 1.f);
}

FSlateIcon UMontageTrack_Sections::GetTrackIcon() const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage");
}

TSharedRef<SWidget> UMontageTrack_Sections::MakeTrackTimelineWidget(
	TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
{
	return SNew(SMGSectionsTrack, TrackModelPtr);
}


// ============================================================
// SMGSectionsTrack
// ============================================================

void SMGSectionsTrack::Construct(const FArguments& InArgs, const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr)
{
	SDopeSheetTrackTimeline::Construct(SDopeSheetTrackTimeline::FArguments(), TrackModelPtr);
}

// ----------------------------------------------------------------
// Hit-test — returns the marker index under LocalPos, or -1
// ----------------------------------------------------------------

int32 SMGSectionsTrack::HitTestMarker(const FGeometry& Geometry, const FVector2D& LocalPos) const
{
	UMontageTrack_Sections* Track = Cast<UMontageTrack_Sections>(TrackModel->ObjPtr);
	if (!Track || !TrackModel->Controller) return -1;

	for (int32 i = 0; i < Track->Sections.Num(); i++)
	{
		UMontageTrackSection_SectionName* Marker = Cast<UMontageTrackSection_SectionName>(Track->Sections[i]);
		if (!Marker) continue;

		const float X = static_cast<float>(TrackModel->Controller->TimeToXOffset(Marker->StartTime, Geometry));
		if (FMath::Abs(LocalPos.X - X) < 8.f)
		{
			return i;
		}
	}
	return -1;
}

// ----------------------------------------------------------------
// Mouse handling
// ----------------------------------------------------------------

FReply SMGSectionsTrack::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && TrackModel && TrackModel->Controller)
	{
		const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		ContextSectionIndex  = HitTestMarker(MyGeometry, LocalPos);
		LastRightClickTime   = TrackModel->Controller->AbsoluteXCoordToTime(MouseEvent.GetScreenSpacePosition().X);
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	return FReply::Unhandled();
}

FReply SMGSectionsTrack::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && HasMouseCapture())
	{
		FMenuBuilder MenuBuilder(true, nullptr);

		if (ContextSectionIndex >= 0)
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

// ----------------------------------------------------------------
// Context menus
// ----------------------------------------------------------------

void SMGSectionsTrack::MakeTrackContextMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AddSection", LOCTEXT("AddSectionHeader", "Add Section"));
	{
		TSharedRef<SEditableTextBox> NameBox =
			SNew(SEditableTextBox)
			.HintText(LOCTEXT("SectionNameHint", "Section name..."))
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type CommitType)
			{
				if (CommitType != ETextCommit::OnEnter || Text.IsEmpty()) return;

				UMontageTrack_Sections* Track = Cast<UMontageTrack_Sections>(TrackModel->ObjPtr);
				if (!Track) return;

				const FScopedTransaction Transaction(LOCTEXT("AddSectionTx", "Add Montage Section"));
				Track->Modify();

				UMontageTrackSection_SectionName* NewMarker = NewObject<UMontageTrackSection_SectionName>(Track);
				NewMarker->SectionName = FName(*Text.ToString());
				NewMarker->StartTime   = LastRightClickTime;
				NewMarker->EndTime     = LastRightClickTime;
				Track->Sections.Add(NewMarker);
				Track->OnTrackPropertiesChanged.Broadcast();
				FSlateApplication::Get().DismissAllMenus();
			});

		MenuBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(4.f, 2.f).FillWidth(1.f)[NameBox],
			FText::GetEmpty(), false);
	}
	MenuBuilder.EndSection();
}

void SMGSectionsTrack::MakeSectionContextMenu(FMenuBuilder& MenuBuilder)
{
	UMontageTrack_Sections* Track = Cast<UMontageTrack_Sections>(TrackModel->ObjPtr);
	if (!Track || !Track->Sections.IsValidIndex(ContextSectionIndex)) return;

	UMontageTrackSection_SectionName* Marker =
		Cast<UMontageTrackSection_SectionName>(Track->Sections[ContextSectionIndex]);
	if (!Marker) return;

	MenuBuilder.BeginSection("SectionOps", FText::FromName(Marker->SectionName));
	{
		// Inline rename
		TSharedRef<SEditableTextBox> NameBox =
			SNew(SEditableTextBox)
			.Text(FText::FromName(Marker->SectionName))
			.OnTextCommitted_Lambda([this, Track, Marker](const FText& Text, ETextCommit::Type CommitType)
			{
				if (CommitType != ETextCommit::OnEnter || Text.IsEmpty()) return;

				const FScopedTransaction Transaction(LOCTEXT("RenameSectionTx", "Rename Montage Section"));
				Marker->Modify();
				Marker->SectionName = FName(*Text.ToString());
				Track->OnTrackPropertiesChanged.Broadcast();
				FSlateApplication::Get().DismissAllMenus();
			});

		MenuBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(4.f, 2.f).FillWidth(1.f)[NameBox],
			LOCTEXT("RenameLabel", "Name"));

		// Delete
		const int32 CapturedIndex = ContextSectionIndex;
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteSection", "Delete Section"),
			FText::GetEmpty(),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
			FUIAction(FExecuteAction::CreateLambda([this, Track, CapturedIndex]()
			{
				const FScopedTransaction Transaction(LOCTEXT("DeleteSectionTx", "Delete Montage Section"));
				Track->Modify();
				Track->Sections.RemoveAt(CapturedIndex);
				Track->OnTrackPropertiesChanged.Broadcast();
			})));
	}
	MenuBuilder.EndSection();
}

// ----------------------------------------------------------------
// Painting
// ----------------------------------------------------------------

int32 SMGSectionsTrack::OnPaint(
	const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 InLayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 LayerId = SDopeSheetTrackTimeline::OnPaint(Args, AllottedGeometry, MyCullingRect,
	                                                  OutDrawElements, InLayerId, InWidgetStyle, bParentEnabled);

	UMontageTrack_Sections* Track = Cast<UMontageTrack_Sections>(TrackModel ? TrackModel->ObjPtr : nullptr);
	if (!Track || !TrackModel->Controller) return LayerId;

	const float            Height    = AllottedGeometry.GetLocalSize().Y;
	const FSlateBrush*     White     = FAppStyle::GetBrush("WhiteBrush");
	const FSlateFontInfo   Font      = FCoreStyle::GetDefaultFontStyle("Bold", 8);

	// Gold for "Default", violet for user-defined sections
	static const FLinearColor DefaultColor(1.0f, 0.85f, 0.2f, 1.0f);
	static const FLinearColor UserColor   (0.75f, 0.55f, 1.0f, 1.0f);

	for (UDopeSheetTrackSection* Raw : Track->Sections)
	{
		UMontageTrackSection_SectionName* Marker = Cast<UMontageTrackSection_SectionName>(Raw);
		if (!Marker) continue;

		const float          X     = static_cast<float>(TrackModel->Controller->TimeToXOffset(Marker->StartTime, AllottedGeometry));
		const FLinearColor   Color = (Marker->SectionName == FName("Default")) ? DefaultColor : UserColor;
		const FText          Label = FText::FromName(Marker->SectionName);

		// Vertical line
		const TArray<FVector2D> Line = { FVector2D(X, 0.f), FVector2D(X, Height) };
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			Line, ESlateDrawEffect::None, Color, true, 1.5f);

		// Flag tab at top (small filled rectangle)
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
			AllottedGeometry.ToPaintGeometry(FVector2D(X, 0.f), FVector2D(64.f, 14.f)),
			White, ESlateDrawEffect::None, Color.CopyWithNewOpacity(0.85f));

		// Section name text inside the tab
		FSlateDrawElement::MakeText(OutDrawElements, LayerId + 2,
			AllottedGeometry.ToPaintGeometry(FVector2D(X + 3.f, 1.f), FVector2D(60.f, 13.f)),
			Label, Font, ESlateDrawEffect::None,
			FLinearColor(0.05f, 0.05f, 0.05f, 1.f));
	}

	return LayerId + 2;
}

#undef LOCTEXT_NAMESPACE
#endif
