#include "MGEdNode.h"
#include "MontageGraphDebugger.h"
#include "MontageGraphEditorTypes.h"
#include "MontageGraph/Nodes/MGNode.h"
#include "MontageGraphEditorSettings.h"
#include "Graph/MontageEdGraph.h"
#include "Graph/Slate/SMGNode.h"

UMGEdNode::UMGEdNode()
{
}

UEdGraphPin* UMGEdNode::GetInputPin() const
{
	if (Pins.Num() == 0)
	{
		return nullptr;
	}

	return Pins[0];
}

UEdGraphPin* UMGEdNode::GetOutputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction == EGPD_Output)
		{
			return Pin;
		}
	}

	return nullptr;
}

void UMGEdNode::DebugEvaluate()
{
	bIsDebugActive = true;
	DebugTotalTime = DebugElapsedTime = 0.f;
}

FLinearColor UMGEdNode::GetBackgroundColor() const
{

	if (!RuntimeNode)
	{
		return FLinearColor::Red;
	}

	const UMontageGraphEditorSettings* Settings   = GetDefault<UMontageGraphEditorSettings>();
	FLinearColor                       FinalColor = GetNodeTitleColor();

	auto MGEdGraph = Cast<UMontageEdGraph>(GetGraph());
	if (MGEdGraph && MGEdGraph->Debugger.IsValid())
	{
		FMontageGraphDebugger* Debugger = MGEdGraph->Debugger.Get();
		if (Debugger && Debugger->IsDebuggerReady())
		{
			FinalColor = (Debugger->SelectedNode == RuntimeNode)
							 ? Settings->DebugSelectedColor
							 : RuntimeNode->GetBackgroundColor();
		}
	}

	if (IsDebugActive())
	{
		// Failsafe check to disable divide by 0
		// const float DebugFadeTime = Settings->DebugFadeTime > 0 ? Settings->DebugFadeTime : 1.f;
		// const float ActiveTime    = WasActiveTime();
		return FLinearColor::LerpUsingHSV(Settings->DebugEvaluatedColor, FinalColor, GetDebugNormalizedTime());
	}


	return FinalColor;
}

void UMGEdNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UMontageGraphPinNames::PinCategory_Transition, UMontageGraphPinNames::PinName_In);
	CreatePin(EGPD_Output, UMontageGraphPinNames::PinCategory_Transition, UMontageGraphPinNames::PinName_Out);
}

void UMGEdNode::PrepareForCopying()
{
	RuntimeNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

FText UMGEdNode::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	if (!RuntimeNode)
	{
		return Super::GetNodeTitle(TitleType);
	}

	return RuntimeNode->GetNodeTitle();
}

void UMGEdNode::AutowireNewNode(UEdGraphPin* FromPin)
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


void UMGEdNode::UpdateWireConnectionParams(FConnectionParams& Params)
{
	const UMontageGraphEditorSettings* Settings  = GetDefault<UMontageGraphEditorSettings>();
	const UMontageEdGraph*        EdGraph = Cast<UMontageEdGraph>(GetGraph());
	if (EdGraph && EdGraph->Debugger.IsValid())
	{
		if (EdGraph->Debugger->SelectedNodes.Contains(this))
		{
			Params.WireColor = Settings->DebugSelectedColor;
			Params.WireThickness += 3.4f * GetDebugNormalizedTime();
			Params.bDrawBubbles = true;
		}

		if (EdGraph->Debugger->EvaluatedNodes.Contains(this))
		{
			Params.WireColor = FLinearColor::LerpUsingHSV(Settings->DebugEvaluatedColor, Params.WireColor, GetDebugNormalizedTime());
			Params.WireThickness *= 3.4f ;
		}
	}
}

void UMGEdNode::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	if (RuntimeNode == nullptr)
	{
		MessageLog.Error<UMGNode*>(*FString::Printf(TEXT("Malformed node detected %s"), *GetName()), RuntimeNode);
	}
}

void UMGEdNode::PostEditUndo()
{
	Super::PostEditUndo();
}

void UMGEdNode::UpdateErrorReporting(USkeletalMesh* InSkeletalMesh, const FText ErrorMessage) const
{
	if (!SlateNode || !RuntimeNode)
	{
		return;
	}

	// check(SlateNode)
	// check(RuntimeNode)
	// check(InSkeletalMesh)

	if (!InSkeletalMesh)
	{
		// Passed in Skeletal Mesh is set to null, clear up if there was any
		SlateNode->SetErrorText(FText::FromString(""));
		return;
	}

	// UAnimationAsset* AnimationAsset = RuntimeNode->GetAnimationAsset();
	// if (!AnimationAsset)
	// {
	// 	// No Anim Asset for this node, clear up error if there was any
	// 	SlateNode->SetErrorText(FText::FromString(""));
	// 	return;
	// }
	//
	// USkeleton* AnimSkeleton = AnimationAsset->GetSkeleton();
	// if (!AnimSkeleton)
	// {
	// 	// Possible ? Anim Asset has no valid skeleton
	// 	SlateNode->SetErrorText(FText::FromString(""));
	// 	return;
	// }

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION == 26
	const USkeleton* MeshSkeleton = InSkeletalMesh ? InSkeletalMesh->Skeleton : nullptr;
#else
	const USkeleton* MeshSkeleton = InSkeletalMesh ? InSkeletalMesh->GetSkeleton() : nullptr;
#endif

	// FText ErrorText;
	// if (!AnimSkeleton->IsCompatible(MeshSkeleton))
	// {
	// 	// Case of skeleton mismatch, set error text with passed in one
	// 	ErrorText = ErrorMessage;
	// }
	//
	// SlateNode->SetErrorText(ErrorText);
}

float UMGEdNode::WasActiveTime() const
{
	return DebugTotalTime - DebugElapsedTime;
}

float UMGEdNode::GetDebugNormalizedTime() const
{
	return DebugElapsedTime / DebugMaxTime;
}

void UMGEdNode::UpdateTime(const float DeltaTime)
{
	if (!RuntimeNode)
	{
		bIsDebugActive = bWasDebugActive = false;
		return;
	}

	if (bIsDebugActive)
	{
		if (DebugTotalTime >= DebugMaxTime)
		{
			bIsDebugActive = false;
			DebugTotalTime = DebugElapsedTime = 0.f;
		}

		DebugTotalTime   = DebugTotalTime + DeltaTime;
		DebugElapsedTime = DebugTotalTime;
	}
}

void UMGEdNode::UpdateTimeRuntimeNodeChanged(const float DeltaTime, const float MaxTime)
{
	// Debugged node changed, mark as was active if we were active
	if (IsDebugActive() && !WasDebugActive())
	{
		bWasDebugActive  = true;
		bIsDebugActive   = false;
		DebugElapsedTime = DebugTotalTime;

		const UMontageEdGraph* EdGraph = Cast<UMontageEdGraph>(GetGraph());
		if (EdGraph && EdGraph->Debugger.IsValid())
		{
			EdGraph->Debugger->EvaluatedNodes.Reset();
		}

	}
	else if (WasDebugActive() && WasActiveTime() >= MaxTime)
	{
		bWasDebugActive = false;
	}

	// Only tick time if we were active and for the duration of max debug time
	if (WasDebugActive() && DebugElapsedTime != 0)
	{
		DebugTotalTime = DebugTotalTime + DeltaTime;
	}
	else
	{
		bIsDebugActive = bWasDebugActive  = false;
		DebugTotalTime = DebugElapsedTime = 0.f;
	}
}
