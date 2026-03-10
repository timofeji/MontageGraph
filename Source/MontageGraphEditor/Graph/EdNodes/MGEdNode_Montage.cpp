#include "MGEdNode_Montage.h"

#include "MontageGraphEditorLog.h"
#include "MontageGraphEditorStyle.h"
#include "MontageGraph/MontageGraph.h"
#include "Tracks/MontageTrack_CollisionCache.h"
#include "Tracks/MontageTrack_GameplayEvent.h"
#include "Tracks/MontageTrack_GameplayState.h"
#include "Tracks/MontageTrack_Sequences.h"
#include "Tracks/MontageTrack_SwordArc.h"
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
	if (RuntimeNode)
	{
		if (UMGNode_Montage* MontageNode = Cast<UMGNode_Montage>(RuntimeNode))
		{
			return MontageNode->BakedData.Montage.Get();
		}
	}
	return nullptr;
}

void UMGEdNode_Montage::OnRenameNode(const FString& NewName)
{
	MontageDisplayName = NewName;
	Modify(true);
}

void UMGEdNode_Montage::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	
	//*MontageNodes always have a sequence track by default*//
	MontageTracks.Add(NewObject<UMontageTrack_Sequences>(this));
}

void UMGEdNode_Montage::MarkStale()
{
	bShouldRegenerate = true;
}

void UMGEdNode_Montage::RegenerateMontage(UMontageGraph* OwnerGraph, UMGNode_Montage* MontageNode)
{
	if (!MontageNode || MontageTracks.IsEmpty())
	{
		UE_LOG(LogMontageGraphEditorError, Error,
		       TEXT("Attempted to Regenerate MontageGraphEdNode with no RuntimeNode[%s]"), *GetNameSafe(this));
		return;
	}

	// Clear previous baked state so stale data from removed tracks never leaks through.
	MontageNode->BakedData = FMGBakedNodeData{};

	// Each track owns its baking logic — adding a new track type requires only a new class
	// with a BakeToNode override; nothing here needs to change.
	for (UDopeSheetTrackBase* Track : MontageTracks)
	{
		if (Track)
		{
			Track->BakeToNode(MontageNode, MontageNode->BakedData, OwnerGraph, MontageDisplayName);
		}
	}

	// Keep the graph-level caches in sync for O(1) runtime lookups by NodeID.
	OwnerGraph->Montages.Add(MontageNode->BakedData.Montage);
	OwnerGraph->CollisionTracers.Add(MontageNode->BakedData.CollisionTracer);
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
