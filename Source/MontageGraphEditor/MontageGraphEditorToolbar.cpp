// Copyright Epic Games, Inc. All Rights Reserved.

#include "MontageGraphEditorToolbar.h"

#include "MontageGraphEditor.h"
#include "MontageGraphEditorCommands.h"
#include "MontageGraphEditorStyle.h"
#include "PersonaModule.h"
#include "Delegates/Delegate.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/SlateDelegates.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Layout/Margin.h"
#include "Math/Vector2D.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Attribute.h"
#include "Styling/AppStyle.h"
#include "Textures/SlateIcon.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealNames.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "WorkflowOrientedApp/SModeWidget.h"

#include "MontageGraph/MontageGraph.h"

class SWidget;

#define LOCTEXT_NAMESPACE "MontageGraphEditorToolbar"

class SMontageGraphModeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMontageGraphModeWidget ) 
		: _CanBeSelected(true)
	{}
		// The currently active mode, used to determine which mode is highlighted
		SLATE_ATTRIBUTE( FName, OnGetActiveMode )

		// The image for the icon
		SLATE_ATTRIBUTE( const FSlateBrush*, IconImage )

		// The delegate that will be called when this widget wants to change the active mode
		SLATE_EVENT( FOnModeChangeRequested, OnSetActiveMode )

		// Can this mode ever be selected?
		SLATE_ATTRIBUTE( bool, CanBeSelected )

		// Slot for the always displayed contents
		SLATE_NAMED_SLOT( FArguments, ShortContents )

		SLATE_ATTRIBUTE( const FSlateBrush*, DirtyMarkerBrush)
		
	SLATE_END_ARGS()
	
	ECheckBoxState GetModeCheckState() const
	{
		if (IsActiveMode())
		{
			return ECheckBoxState::Checked;
		}
		else
		{
			return ECheckBoxState::Unchecked;
		}
	}

	bool IsActiveMode() const
	{
		return OnGetActiveMode.Get() == ThisMode;
	}

	void OnModeTabClicked(ECheckBoxState CheckState)
	{
		// Try to change the mode
		if (!IsActiveMode() && (CanBeSelected.Get() == true))
		{
			OnSetActiveMode.ExecuteIfBound(ThisMode);
		}
	}

	void Construct(const FArguments& InArgs, const FText& InText, const FName InMode)
	{

	// Copy arguments
	ModeText = InText;
	ThisMode = InMode;
	OnGetActiveMode = InArgs._OnGetActiveMode;
	CanBeSelected = InArgs._CanBeSelected;
	OnSetActiveMode = InArgs._OnSetActiveMode;

	// Load resources
	InactiveModeBorderImage = FAppStyle::GetBrush("ModeSelector.ToggleButton.Normal");
	ActiveModeBorderImage = FAppStyle::GetBrush("ModeSelector.ToggleButton.Pressed");
	HoverBorderImage = FAppStyle::GetBrush("ModeSelector.ToggleButton.Hovered");
	
	TSharedRef<SHorizontalBox> InnerRow = SNew(SHorizontalBox);

	FMargin IconPadding(4.0f, 0.0f, 4.0f, 0.0f);
	FMargin BodyPadding(0.0f, 0.0f, 0.0f, 0.0f);
	
	if (InArgs._IconImage.IsSet())
	{
		InnerRow->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(IconPadding)
			[
				SNew(SImage)
				.Image(InArgs._IconImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
			];
	}

	// Label + content
	InnerRow->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(BodyPadding)
		[
			SNew(SVerticalBox)

			// Mode 'tab'
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				//Mode Name
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(ModeText)
				]

				//Dirty flag
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(3)
				[
					SNew(SImage)
					.Image(InArgs._DirtyMarkerBrush)
				]
			]

			// Body of 'ribbon'
			+SVerticalBox::Slot()
			.AutoHeight()
			[

				InArgs._ShortContents.Widget
			]
		];


	// Create the widgets
	ChildSlot
	.VAlign(VAlign_Fill)
	[
		SNew(SCheckBox)
		.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
		.IsChecked(this, &SMontageGraphModeWidget::GetModeCheckState)
		.OnCheckStateChanged(this, &SMontageGraphModeWidget::OnModeTabClicked)
		[
			InnerRow
		]
	];

	SetEnabled(CanBeSelected);
	}



private:
	// The active mode of this group
	TAttribute<FName> OnGetActiveMode;

	// The delegate to call when this mode is selected
	FOnModeChangeRequested OnSetActiveMode;

	// Can this mode be selected?
	TAttribute<bool> CanBeSelected;

	// The text representation of this mode
	FText ModeText;

	// The mode this widget is representing
	FName ThisMode;

	// Border images
	const FSlateBrush* ActiveModeBorderImage;
	const FSlateBrush* InactiveModeBorderImage;
	const FSlateBrush* HoverBorderImage;

};

void FMontageGraphEditorToolbar::AddModesToolbar(TSharedPtr<FExtender> Extender)
{
	check(MGEditor.IsValid());
	TSharedPtr<FMontageGraphEditor> MGEditorPtr = MGEditor.Pin();

	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::First,
		MGEditorPtr->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FMontageGraphEditorToolbar::FillModesToolbar));
	
}

void FMontageGraphEditorToolbar::AddDebuggerToolbar(TSharedPtr<FExtender> Extender)
{
	check(MGEditor.IsValid());
	TSharedPtr<FMontageGraphEditor> MontageGraphEditorPtr = MGEditor.Pin();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, MontageGraphEditorPtr->GetToolkitCommands(),
	                                     FToolBarExtensionDelegate::CreateSP(
		                                     this, &FMontageGraphEditorToolbar::FillDebuggerToolbar));
	MontageGraphEditorPtr->AddToolbarExtender(ToolbarExtender);
	
	
	// // setup toolbar
	// struct Local
	// {
	// 	static void FillToolbar(FToolBarBuilder& ToolbarBuilder, TWeakPtr<FMontageGraphEditor> MGEditor)
	// 	{
	// 		TSharedPtr<FMontageGraphEditor> MGEditorPtr = MGEditor.Pin();
	//
	// 		const bool bCanShowDebugger = MGEditorPtr ->IsDebuggerReady();
	// 		if (bCanShowDebugger)
	// 		{
	// 			TSharedRef<SWidget> SelectionBox = SNew(SComboButton)
	// 				.OnGetMenuContent( MGEditorPtr .Get(), &FMontageGraphEditor::OnGetDebuggerActorsMenu )
	// 				.ButtonContent()
	// 				[
	// 					SNew(STextBlock)
	// 					.ToolTipText( LOCTEXT("SelectDebugActor", "Pick actor to debug") )
	// 					.Text(MGEditorPtr .Get(), &FMontageGraphEditor::GetDebuggerActorDesc )
	// 				];
	//
	// 			ToolbarBuilder.BeginSection("CachedState");
	// 			{
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().BackOver);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().BackInto);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().ForwardInto);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().ForwardOver);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().StepOut);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().OpenParentTree);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().OpenSubtree);
	// 			}
	// 			ToolbarBuilder.EndSection();
	// 			ToolbarBuilder.BeginSection("World");
	// 			{
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().PausePlaySession);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().ResumePlaySession);
	// 				ToolbarBuilder.AddToolBarButton(FBTDebuggerCommands::Get().StopPlaySession);
	// 				ToolbarBuilder.AddSeparator();
	// 				ToolbarBuilder.AddWidget(SelectionBox);
	// 			}
	// 			ToolbarBuilder.EndSection();
	// 		}
	// 	}
	// };
	//
	// TSharedPtr<FMontageGraphEditor> MontageGraphEditorPtr = MGEditor.Pin();
	//
	// TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	// ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, MontageGraphEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateStatic( &Local::FillToolbar, MGEditor ));
	// MontageGraphEditorPtr->AddToolbarExtender(ToolbarExtender);
}


void FMontageGraphEditorToolbar::AddGameplayToolbar(TSharedPtr<FExtender> Extender)
{
	

	
	check(MGEditor.IsValid());
	TSharedPtr<FMontageGraphEditor> MontageGraphEditorPtr = MGEditor.Pin();

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, MontageGraphEditorPtr->GetToolkitCommands(), FToolBarExtensionDelegate::CreateSP( this, &FMontageGraphEditorToolbar::FillGameplayToolbar ));
	MontageGraphEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FMontageGraphEditorToolbar::SetCreateActionsEnabled(const bool bActionsEnabled)
{
	bCreateActionsEnabled = bActionsEnabled;
}

void FMontageGraphEditorToolbar::FillModesToolbar(FToolBarBuilder& ToolbarBuilder)
{
	check(MGEditor.IsValid());
	TSharedPtr<FMontageGraphEditor> MontageGraphEditorPtr = MGEditor.Pin();

	TAttribute<FName> GetActiveMode(MontageGraphEditorPtr.ToSharedRef(), &FMontageGraphEditor::GetCurrentMode);
	FOnModeChangeRequested SetActiveMode = FOnModeChangeRequested::CreateSP(
		MontageGraphEditorPtr.ToSharedRef(), &FMontageGraphEditor::SetCurrentMode);

	MontageGraphEditorPtr->AddToolbarWidget(
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
		.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(5, 0, 5, 0)
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SMontageGraphModeWidget, FMontageGraphEditorModes::GetLocalizedMode( FMontageGraphEditorModes::Rules ),
				     FMontageGraphEditorModes::Rules)
				.OnGetActiveMode(GetActiveMode)
				.OnSetActiveMode(SetActiveMode)
				.CanBeSelected(MontageGraphEditorPtr.Get(), &FMontageGraphEditor::CanAccessGameplayMode)
				.ToolTipText(LOCTEXT("MontageGraphEditorModeButtonTooltip_gameplay", "Switch to Gameplay Edit Mode"))
				.IconImage(FAppStyle::GetBrush("BTEditor.SwitchToBehaviorTreeMode"))
			]
			+ SHorizontalBox::Slot()
			.Padding(5, 0, 5, 0)
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[

				SNew(SMontageGraphModeWidget, FMontageGraphEditorModes::GetLocalizedMode( FMontageGraphEditorModes::Blends ),
				     FMontageGraphEditorModes::Blends)
				.OnGetActiveMode(GetActiveMode)
				.OnSetActiveMode(SetActiveMode)
				.CanBeSelected(MontageGraphEditorPtr.Get(), &FMontageGraphEditor::CanAccessBlendsMode)
				.ToolTipText(LOCTEXT("MontageGraphEditorModeButtonTooltip_blend", "Switch to Blend Edit Mode"))
				.IconImage(FAppStyle::GetBrush("BTEditor.SwitchToBlackboardMode"))
			]
			+ SHorizontalBox::Slot()
			.Padding(5, 0, 5, 0)
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[

				SNew(SMontageGraphModeWidget, FMontageGraphEditorModes::GetLocalizedMode( FMontageGraphEditorModes::Debug ),
				     FMontageGraphEditorModes::Debug)
				.OnGetActiveMode(GetActiveMode)
				.OnSetActiveMode(SetActiveMode)
				.CanBeSelected(MontageGraphEditorPtr.Get(), &FMontageGraphEditor::CanAccessBlendsMode)
				.ToolTipText(LOCTEXT("MontageGraphEditorModeButtonTooltip_debug", "Switch to Debug Mode"))
				.IconImage(FAppStyle::GetBrush("Debug"))
			]
		]
	);

	
	// Right side padding
	MontageGraphEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(40.0f, 0.0f)));
}

void FMontageGraphEditorToolbar::FillDebuggerToolbar(FToolBarBuilder& ToolbarBuilder)
{
	// const UMontageGraphEdGraph* EditorGraph = Cast<UMontageGraphEdGraph>(
	// 	GraphBeingEdited->EditorGraph);
	//
	// const TSharedRef<SWidget> SelectionBox = SNew(SComboButton)
	// 	.OnGetMenuContent(this, &FMontageGraphEditor::OnGetDebuggerActorsMenu)
	// 	.ButtonContent()
	// 	[
	// 		SNew(STextBlock)
	// 		.ToolTipText(LOCTEXT("SelectDebugActor", "Pick actor to debug"))
	// 		.Text(EditorGraph && EditorGraph->Debugger.IsValid()
	// 			      ? FText::FromString(EditorGraph->Debugger->GetDebuggedInstanceDesc())
	// 			      : FText::GetEmpty())
	// 	];
	//
	// ToolbarBuilder.BeginSection("Debugger");
	// {
	// 	ToolbarBuilder.AddWidget(SelectionBox);
	// 	// ToolBarBuilder.AddWidget(DebugModeComboBox);
	// }
	// ToolbarBuilder.EndSection();
}

void FMontageGraphEditorToolbar::FillGameplayToolbar(FToolBarBuilder& ToolbarBuilder)
{
	if (!bCreateActionsEnabled)
	{
		return;
	}
	
	// check(MGEditor.IsValid());
	// TSharedPtr<FMontageGraphEditor> MontageGraphEditorPtr = MGEditor.Pin();
	//
	// if (MontageGraphEditorPtr->GetCurrentMode() == FMontageGraphEditorModes::Gameplay)
	// {
	// 	const auto MontageGraphEditorCommands = FMontageGraphEditorCommands::Get();
	// 	ToolbarBuilder.BeginSection("Gameplay");
	// 	{
	// 		const FText NewTaskLabel = LOCTEXT("RegenTracers_Label", "Regenerate Tracers");
	// 		const FText NewTaskTooltip = LOCTEXT("RegenerateTracersTooltip",
	// 		                                     "Regenerates Tracers for all montage nodes");
	// 		const FSlateIcon NewTaskIcon = FSlateIcon(FMontageGraphEditorStyle::Get().GetStyleSetName(),
	// 		                                          "MontageGraph.Icon.RegenerateTracers");
	//
	//
	// 		ToolbarBuilder.AddToolBarButton(
	// 			MontageGraphEditorCommands.RegenerateTracers,
	// 			NAME_None,
	// 			NewTaskLabel,
	// 			NewTaskTooltip,
	// 			NewTaskIcon
	// 		);
	// 	}
	// 	ToolbarBuilder.EndSection();
	// }
}

#undef LOCTEXT_NAMESPACE
