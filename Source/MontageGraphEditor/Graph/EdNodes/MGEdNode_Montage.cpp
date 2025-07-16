#include "MGEdNode_Montage.h"

#include "MontageGraphEditorLog.h"
#include "MontageGraphEditorStyle.h"
#include "MontageGraph/MontageGraph.h"
#include "Tracks/MontageTrack_CollisionCache.h"
#include "Tracks/MontageTrack_Sequences.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"

#define LOCTEXT_NAMESPACE "MGEdNode_Montage"

const FName UMGEdNode_Montage::DefaultSlotName("DefaultSlot");

UMGEdNode_Montage::UMGEdNode_Montage()
{
	bCanRenameNode = true;
	
}

FLinearColor UMGEdNode_Montage::GetNodeTitleColor() const
{
	return FLinearColor(FColor::FromHex(TEXT("#000E8ED6")));
}

FSlateIcon UMGEdNode_Montage::GetIconAndTint(FLinearColor& OutColor) const
{
	
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.AnimMontage");
}

UAnimMontage* UMGEdNode_Montage::GetMontage()
{
	UAnimMontage* AnimMontage = nullptr;

	if (RuntimeNode)
	{
		if (UMGNode_Montage* MontageNode = Cast<UMGNode_Montage>(RuntimeNode))
		{
			// AnimMontage = MontageNode->Montage;
		}
	}

	return AnimMontage;
}

void UMGEdNode_Montage::OnRenameNode(const FString& NewName)
{
	MontageDisplayName = NewName;
	Modify(true);
}

void UMGEdNode_Montage::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();


	//*MontageNodes always have a sequence and collision tracks by default*//
	MontageTracks.Add(NewObject<UMontageTrack_Sequences>(this));
	MontageTracks.Add(NewObject<UMontageTrack_CollisionCache>(this));
}

void UMGEdNode_Montage::MarkStale()
{
	bShouldRegenerate = true;
}

void UMGEdNode_Montage::RegenerateMontage(UMontageGraph* OwnerGraph)
{
	if (!RuntimeNode || MontageTracks.Num() < 2)
	{
		UE_LOG(LogMontageGraphEditorError, Error,
		       TEXT("Attempted to Regenerate MontageGraphEdNode with no RuntimeNode[%s]"), *GetNameSafe(this));
		return;
	}


	UAnimMontage* NewMontage = MontageTracks[0]->CreateNewDataObject<UAnimMontage>(
		OwnerGraph, FName(MontageDisplayName + "_Montage"));
	UCollisionTracer* NewTracerData = MontageTracks[1]->CreateNewDataObject<UCollisionTracer>(
		NewMontage , FName(MontageDisplayName + "_Collision"));

	OwnerGraph->Montages.Add(NewMontage);
	OwnerGraph->CollisionTracers.Add(NewTracerData);
}

void UMGEdNode_Montage::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin)
	{
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

FText UMGEdNode_Montage::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return MontageDisplayName.IsEmpty() ? LOCTEXT("MontageNodeTitle", "MontageNode") : FText::FromString(MontageDisplayName);
}

FText UMGEdNode_Montage::GetTooltipText() const
{
	return GetNodeTitle(ENodeTitleType::Type::FullTitle);
}

void UMGEdNode_Montage::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Consider validation error for this node
	Super::ValidateNodeDuringCompilation(MessageLog);
}
#undef LOCTEXT_NAMESPACE
