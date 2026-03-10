// Copyright Drop Games Inc.


#include "MontageTrack_BlendLinks.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "MontageGraphEditorLog.h"
#include "MontageGraphEditorStyle.h"
#include "Graph/EdNodes/MGEdNode_Montage.h"
#include "MontageGraph/MontageGraph.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"

#define LOCTEXT_NAMESPACE "SMGBlendLinksTrack"

void SMGBlendLinksTrack::Construct(const FArguments& InArgs, const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr)
{
	TrackModel = TrackModelPtr;

	if (TrackModel.IsValid() && TrackModel->ObjPtr)
	{
		TrackModel->ObjPtr->OnTrackPropertiesChanged.AddRaw(
			this, &SDopeSheetTrackTimeline::RegenerateSections);
	}


	RegenerateSections();
}


void SMGBlendLinksTrack::MakeTrackContextMenu(FMenuBuilder& ContextMenuBuilder)
{

	static constexpr float  ThumbnailIconSize       = 64.0f;
	static constexpr uint32 ThumbnailIconResolution = 64;

	if (!(TrackModel) || !(TrackModel->ObjPtr))
	{
		return;
	}

	ContextMenuBuilder.BeginSection(TEXT("BlendLink"), LOCTEXT("BlendLink", "BlendLink"));
	{
		FUIAction AddLinkAction(FExecuteAction::CreateLambda([this]() {
			if (auto BlendLinksTrack = Cast<UMontageTrack_BlendLinks>(TrackModel->ObjPtr))
			{
				// BlendLinksTrack->AddLink();
			}
		}));
		ContextMenuBuilder.AddMenuEntry(FText::FromString("Add BlendLink"), TAttribute<FText>(),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage"),
			AddLinkAction);
	}
	ContextMenuBuilder.EndSection();

}


TSharedRef<SWidget> UMontageTrack_BlendLinks::MakeTrackTimelineWidget(
	TSharedPtr<FDopeSheetTrackViewModel> TrackModelPtr) const
{
	return SNew(SMGBlendLinksTrack, TrackModelPtr);
}

void UMontageTrack_BlendLinks::AddLink()
{
}

void UMontageTrack_BlendLinks::KeyTimeRange(float SelectionStartFrame, float SelectionEndFrame)
{
	const FScopedTransaction Transaction(NSLOCTEXT("DopeSheet", "RemoveDopeSheetSection_Transaction",
	                                               "Add Timeline section"));

	Modify(true);

	auto NewSection       = NewObject<UMontageTrackSection_LinkBlend>(this);
	NewSection->StartTime = SelectionStartFrame;
	NewSection->EndTime   = SelectionEndFrame;
	Sections.Add(NewSection);

	OnTrackPropertiesChanged.Broadcast();
}


UMontageTrack_BlendLinks::UMontageTrack_BlendLinks(const FObjectInitializer& ObjectInitializer)
{
	FDopeSheetKey DefaultBlendKey = FDopeSheetKey();
	Keys.Add(DefaultBlendKey);
	
	
	bShouldDrawCells = true;
}
//
// UObject* UMontageTrack_BlendLinks::GenerateNewDataAsset(UObject* Outer, FName Name)
// {
// 	// UMontageGraph*    OwnerGraph      = Cast<UMontageGraph>(Outer->GetOuter());
// 	UMontageGraphBlendLink* LinkData      = NewObject<UMontageGraphBlendLink>(OwnerGraph, Name, RF_Public | RF_Standalone);
// }

const FText UMontageTrack_BlendLinks::GetTrackName() const
{
	if (Sections.Num() > 0)
	{
		auto LinkSection = Cast<UMontageTrackSection_LinkBlend>(Sections[0]);
		if (LinkSection && LinkSection->TargetNode)
		{
			return LinkSection->TargetNode->GetNodeTitle(ENodeTitleType::Type::FullTitle);
		}
	}
	return FText::FromString("Link");
}

const FLinearColor UMontageTrack_BlendLinks::GetTrackColor() const
{
	return FColor(49, 73, 44, 255);
}

const FLinearColor UMontageTrack_BlendLinks::GetSectionColor() const
{
	return FColor(96, 104, 147, 255);
}

FSlateIcon UMontageTrack_BlendLinks::GetTrackIcon() const
{
	return FSlateIcon(FMontageGraphEditorStyle::GetStyleSetName(), "MontageGraph.Node.Icon.Selector");
}


void UMontageTrackSection_LinkBlendDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	UMontageGraph* Graph = nullptr;
	if (Objects.Num() == 1)
	{
		EditedSection      = Cast<UMontageTrackSection_LinkBlend>(Objects[0].Get());
		UObject* Outermost = EditedSection->GetOutermostObject();
		Graph              = Cast<UMontageGraph>(Outermost);
	}

	if (!Graph)
	{
		return;
	}

	for (auto Node : Graph->EditorGraph->Nodes)
	{
		if (UMGEdNode_Montage* MontageNode = Cast<UMGEdNode_Montage>(Node))
		{
			NodeOptions.Add(MontageNode);
		}
	}

	TSharedRef<IPropertyHandle> TargetNodeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UMontageTrackSection_LinkBlend, TargetNode));
	DetailBuilder.HideProperty(TargetNodeHandle);

	// // Add our custom combo box
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Link Target");
	Category.AddCustomRow(LOCTEXT("TargetNodeFilter", "Target Node"))
			.NameContent()
		[
			TargetNodeHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(200.f)
		[
			SNew(SComboBox<UMGEdNode_Montage*>)
			.OptionsSource(&NodeOptions)
			.OnGenerateWidget(this, &UMontageTrackSection_LinkBlendDetails::MakeNodeWidget)
			.OnSelectionChanged(this, &UMontageTrackSection_LinkBlendDetails::OnTargetNodeChanged)
			[
				SNew(STextBlock)
				.Text(this, &UMontageTrackSection_LinkBlendDetails::GetCurrentTargetNodeText)
			]
		];

}

void UMontageTrackSection_LinkBlendDetails::OnTargetNodeChanged(UMGEdNode_Montage* NewSelection, ESelectInfo::Type SelectInfo)
{
	if (IsValid(EditedSection))
	{
		EditedSection->Modify();
		EditedSection->TargetNode = NewSelection ? NewSelection : nullptr;
	}
}

FText UMontageTrackSection_LinkBlendDetails::GetCurrentTargetNodeText() const
{
	if (!IsValid(EditedSection))
		return LOCTEXT("Invalid", "Invalid");

	if (EditedSection->TargetNode)
		return EditedSection->TargetNode->GetNodeTitle(ENodeTitleType::Type::FullTitle);

	return LOCTEXT("None", "None");
}

TSharedRef<SWidget> UMontageTrackSection_LinkBlendDetails::MakeNodeWidget(UMGEdNode_Montage* InNode)
{
	return SNew(STextBlock)
		.Text(InNode ? InNode->GetNodeTitle(ENodeTitleType::Type::FullTitle) : FText::FromString(TEXT("None")));
}

#undef LOCTEXT_NAMESPACE
