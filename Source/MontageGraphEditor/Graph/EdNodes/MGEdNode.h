
#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"
#include "MontageGraph/Nodes/MGEdge.h"
#include "MGEdNode.generated.h"

class UMGNode;
class SMGNode;

/** Shared Base class for all Editor Graph nodes */
UCLASS()
class UMGEdNode: public UEdGraphNode
{
	GENERATED_BODY()

public:
	
	UMGEdNode();

	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadWrite)
	UMGNode* RuntimeNode;

	virtual UMGNode* GetRuntimeNode() { return RuntimeNode; }

	

	virtual UEdGraphPin* GetInputPin() const;
	virtual UEdGraphPin* GetOutputPin() const;

	//~ Begin UEdGraphNode Interface
	// Make those noop like in UEdGraphNode. UK2Node does CastChecked on Graph Schema to be UEdGraphSchema_K2 and don't want to make schema K2.
	// Nodes are now inheriting from UK2Node to avoid ensure condition on startup with pure check (see #3)
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override ;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override {}
	//~ End UEdGraphNode Interface
	
	void UpdateWireConnectionParams(FConnectionParams& Params);

	SMGNode* SlateNode;

	
	void DebugEvaluate();

	virtual FLinearColor GetBackgroundColor() const;

	//~ UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual void PrepareForCopying() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
	//~ End UEdGraphNode interface


	bool IsDebugActive() const { return bIsDebugActive; }
	bool WasDebugActive() const { return bWasDebugActive; }
	float GetDebugTotalTime() const { return DebugTotalTime; }
	float GetEvaluationTime(){return EvaluatedInTime;}
	float GetDebugNormalizedTime() const;

	/** Called from associated Slate Node widget tick to pass in time ticks and update debug information. */
	virtual void UpdateTime(float DeltaTime);

	/** Updates Error Text based on Skeletal Mesh passed in, checking if anim asset defined with this node has a skeleton compatible with passed in Mesh */
	void UpdateErrorReporting(USkeletalMesh* InSkeletalMesh, FText ErrorMessage) const;

protected:
	bool bIsDebugActive = false;
	bool bWasDebugActive = false;
	
	
	const float DebugMaxTime = 2.f;

	/** Resets when debug active states change. */
	float DebugTotalTime = 0.f;

	/** The amount of time the observed node was active */
	float DebugElapsedTime = 0.f;

	/*Time it took to find node*/
	float EvaluatedInTime = 0.f;

	/** Returns amount of time this node has been marked as was active */
	float WasActiveTime() const;

	/** Update time logic for when runtime node is not the same as debugged node (node changed) */
	void UpdateTimeRuntimeNodeChanged(float DeltaTime, float MaxTime);

};
