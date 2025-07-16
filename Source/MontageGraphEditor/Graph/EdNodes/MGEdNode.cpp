
#include "MGEdNode.h"

#include "MontageGraphEditorTypes.h"
#include "MontageGraph/Nodes/MGNode.h"
#include "MontageGraphEditorSettings.h"
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
		return FLinearColor::Black;
	}

	const UMontageGraphEditorSettings* Settings = GetDefault<UMontageGraphEditorSettings>();
	const FLinearColor DebugActiveColor = Settings->DebugActiveColor;
	const FLinearColor DefaultColor = RuntimeNode->GetBackgroundColor();

	if (IsDebugActive())
	{
		return DebugActiveColor;
	}

	// Failsafe check to disable divide by 0
	const float DebugFadeTime = Settings->DebugFadeTime > 0 ? Settings->DebugFadeTime : 1.f;
	const float ActiveTime = WasActiveTime();
	if (WasDebugActive() && ActiveTime < 3.f)
	{
		return FLinearColor::LerpUsingHSV(DebugActiveColor, DefaultColor, ActiveTime / DebugFadeTime);
	}

	return DefaultColor;
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

FLinearColor DefaultWireColor(1.00f, 1.00f, 1.00f, 0.43f);
FLinearColor DebugWireColor(1.00f, 0.00f, 0.06f, 0.43f);
FLinearColor UMGEdNode::GetWireColor()
{
	return bIsDebugActive ? DebugWireColor : DefaultWireColor;
}

void UMGEdNode::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	// TODO: Move most of the editor stuff into Developer module (or UncookedOnly). See how ControlRig is done regarding this
	// Super::ValidateNodeDuringCompilation(MessageLog);

	// const bool bIsSequence = RuntimeNode->IsA(UMontageGraphNodeSequence::StaticClass());
	//
	// if (bIsSequence)
	// {
	// 	MessageLog.Warning(TEXT("@@ Using sequences combo nodes is not supported in networked environment. Try using Montages instead."), this);
	// }
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


float UMGEdNode::GetDebugNormalizedTime()
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
	

	if(bIsDebugActive)
	{
		if (DebugTotalTime >= DebugMaxTime)
		{
			bIsDebugActive = false;
			DebugTotalTime = DebugElapsedTime = 0.f;
		}
		
		DebugTotalTime = DebugTotalTime + DeltaTime;
		DebugElapsedTime = DebugTotalTime;
	}
}


void UMGEdNode::UpdateTimeRuntimeNodeChanged(const float DeltaTime, const float MaxTime)
{
	// Debugged node changed, mark as was active if we were active
	if (IsDebugActive() && !WasDebugActive())
	{
		bWasDebugActive = true;
		bIsDebugActive = false;
		DebugElapsedTime = DebugTotalTime;
		// DebugTotalTime = 0.f;
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
		bIsDebugActive = bWasDebugActive = false;
		DebugTotalTime = DebugElapsedTime = 0.f;
	}
}



const UMGNode* UMGEdNode::GetDebuggedNode() const
{
	return nullptr;
}
