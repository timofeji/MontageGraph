#include "MontageTrack_GameplayState.h"

#include "MontageGraphEditorStyle.h"
#include "ScopedTransaction.h"
#include "DopeSheet/DopeSheetController.h"
#include "DopeSheet/Tracks/DopeSheetTrackViewModel.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "MontageGraph/Notifies/MG_AnimNotifyState_LooseTag.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Animation/AnimMontage.h"

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "MontageTrack_GameplayState"

// ============================================================
// SMGGameplayStateTrack
// ============================================================

void SMGGameplayStateTrack::Construct(const FArguments& InArgs,
                                      const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr)
{
	SDopeSheetTrackTimeline::Construct(SDopeSheetTrackTimeline::FArguments(), TrackModelPtr);
}

// ----------------------------------------------------------------
// Track context menu — right-click on empty space to add a state section
// ----------------------------------------------------------------

void SMGGameplayStateTrack::MakeTrackContextMenu(FMenuBuilder& MenuBuilder)
{
	if (!TrackModel || !TrackModel->ObjPtr || !TrackModel->Controller) { return; }

	// Cache right-click time before the lambda captures it
	LastRightClickTime = TrackModel->Controller->AbsoluteXCoordToTime(
		FSlateApplication::Get().GetCursorPos().X);

	MenuBuilder.BeginSection("AddState", LOCTEXT("AddStateHeader", "Add State"));
	{
		// Quick "Add 1s State" entry — creates a 1-second section at the click position
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AddStateEntry", "Add State Section (1s)"),
			LOCTEXT("AddStateEntryTip", "Insert a 1-second state section at this time"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				UMontageTrack_GameplayState* Track = Cast<UMontageTrack_GameplayState>(TrackModel->ObjPtr);
				if (!Track) { return; }

				const FScopedTransaction Tx(LOCTEXT("AddStateTx", "Add Gameplay State"));
				Track->Modify();

				UMontageTrackSection_GameplayState* NewSection =
					NewObject<UMontageTrackSection_GameplayState>(Track);
				NewSection->StartTime = LastRightClickTime;
				NewSection->EndTime   = LastRightClickTime + 1.0;
				Track->Sections.Add(NewSection);
				Track->OnTrackPropertiesChanged.Broadcast();
			})));
	}
	MenuBuilder.EndSection();
}

// ----------------------------------------------------------------
// Section context menu — edit tags + delete
// ----------------------------------------------------------------

void SMGGameplayStateTrack::MakeSectionContextMenu(FMenuBuilder& MenuBuilder)
{
	UMontageTrack_GameplayState* Track = Cast<UMontageTrack_GameplayState>(
		TrackModel ? TrackModel->ObjPtr : nullptr);
	if (!Track || !Track->Sections.IsValidIndex(SelectedSectionIndex)) { return; }

	UMontageTrackSection_GameplayState* S = Cast<UMontageTrackSection_GameplayState>(
		Track->Sections[SelectedSectionIndex]);
	if (!S) { return; }

	MenuBuilder.BeginSection("StateOps", LOCTEXT("StateOpsHeader", "Gameplay State"));
	{
		// Display existing tags
		if (!S->LooseTags.IsEmpty())
		{
			const FText TagSummary = FText::FromString(S->LooseTags.ToStringSimple(false));
			MenuBuilder.AddWidget(
				SNew(STextBlock)
				.Text(TagSummary)
				.ColorAndOpacity(FLinearColor(0.7f, 1.f, 0.7f, 1.f)),
				LOCTEXT("ActiveTags", "Active Tags"));
		}

		// Inline "add tag" text box
		TSharedRef<SEditableTextBox> AddTagBox =
			SNew(SEditableTextBox)
			.HintText(LOCTEXT("AddTagHint", "Add tag..."))
			.OnTextCommitted_Lambda([this, Track, S](const FText& Text, ETextCommit::Type CommitType)
			{
				if (CommitType != ETextCommit::OnEnter || Text.IsEmpty()) { return; }
				const FGameplayTag NewTag = FGameplayTag::RequestGameplayTag(
					FName(*Text.ToString()), false);
				if (!NewTag.IsValid()) { return; }

				const FScopedTransaction Tx(LOCTEXT("AddTagToStateTx", "Add Tag to State"));
				S->Modify();
				S->LooseTags.AddTag(NewTag);
				Track->OnTrackPropertiesChanged.Broadcast();
				FSlateApplication::Get().DismissAllMenus();
			});

		MenuBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(4.f, 2.f).FillWidth(1.f)[AddTagBox],
			LOCTEXT("AddTagLabel", "Add Tag"));

		// Clear all tags
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ClearTags", "Clear All Tags"),
			FText::GetEmpty(),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
			FUIAction(FExecuteAction::CreateLambda([this, Track, S]()
			{
				const FScopedTransaction Tx(LOCTEXT("ClearTagsTx", "Clear State Tags"));
				S->Modify();
				S->LooseTags.Reset();
				Track->OnTrackPropertiesChanged.Broadcast();
			})));

		// Delete section
		const int32 CapturedIdx = SelectedSectionIndex;
		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteState", "Delete Section"),
			FText::GetEmpty(),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
			FUIAction(FExecuteAction::CreateLambda([this, Track, CapturedIdx]()
			{
				const FScopedTransaction Tx(LOCTEXT("DeleteStateTx", "Delete Gameplay State"));
				Track->Modify();
				Track->Sections.RemoveAt(CapturedIdx);
				Track->OnTrackPropertiesChanged.Broadcast();
			})));
	}
	MenuBuilder.EndSection();
}

// ============================================================
// UMontageTrack_GameplayState
// ============================================================

UMontageTrack_GameplayState::UMontageTrack_GameplayState()
{
	CollectionName   = FName("Gameplay");
	bAllowRename     = false;
	bShouldDrawCells = true;
}

void UMontageTrack_GameplayState::KeyTimeRange(float SelectionStartTime, float SelectionEndTime)
{
	const FScopedTransaction Transaction(LOCTEXT("AddStateSectionTx", "Add State Section"));
	Modify(true);

	UMontageTrackSection_GameplayState* NewSection =
		NewObject<UMontageTrackSection_GameplayState>(this);
	NewSection->StartTime = static_cast<double>(SelectionStartTime);
	NewSection->EndTime   = static_cast<double>(SelectionEndTime);
	Sections.Add(NewSection);

	OnTrackPropertiesChanged.Broadcast();
}

void UMontageTrack_GameplayState::BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
                                              UMontageGraph* OwnerGraph, const FString& DisplayName)
{
	if (!BakedData.Montage || Sections.IsEmpty()) { return; }

	for (UDopeSheetTrackSection* Raw : Sections)
	{
		UMontageTrackSection_GameplayState* StateSection =
			Cast<UMontageTrackSection_GameplayState>(Raw);
		if (!StateSection || StateSection->LooseTags.IsEmpty()) { continue; }

		const float StartTime = static_cast<float>(StateSection->StartTime);
		const float Duration  = static_cast<float>(StateSection->EndTime - StateSection->StartTime);
		if (Duration <= 0.f) { continue; }

		UMG_AnimNotifyState_LooseTag* State = NewObject<UMG_AnimNotifyState_LooseTag>(BakedData.Montage);
		State->Tags = StateSection->LooseTags;

		FAnimNotifyEvent NewNotify;
		NewNotify.NotifyStateClass = State;
		NewNotify.NotifyName       = FName("LooseTag");
		NewNotify.Duration         = Duration;
		NewNotify.TrackIndex       = 2;
		NewNotify.Link(BakedData.Montage, StartTime);

		BakedData.Montage->Notifies.Add(NewNotify);
	}

	FProperty* Prop = nullptr;
	FPropertyChangedEvent Evt(Prop);
	BakedData.Montage->PostEditChangeProperty(Evt);
}

const FText UMontageTrack_GameplayState::GetTrackName() const
{
	return LOCTEXT("TrackName", "Gameplay State");
}

const FLinearColor UMontageTrack_GameplayState::GetTrackColor() const
{
	return FLinearColor(0.2f, 0.85f, 0.75f, 1.f);
}

const FLinearColor UMontageTrack_GameplayState::GetSectionColor() const
{
	return FLinearColor(0.2f, 0.85f, 0.75f, 1.f);
}

FSlateIcon UMontageTrack_GameplayState::GetTrackIcon() const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.GameplayAbility");
}

TSharedRef<SWidget> UMontageTrack_GameplayState::MakeTrackTimelineWidget(
	TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
{
	return SNew(SMGGameplayStateTrack, TrackModelPtr);
}

#undef LOCTEXT_NAMESPACE
#endif
