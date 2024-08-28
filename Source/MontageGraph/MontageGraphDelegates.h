#pragma once

#include "CoreMinimal.h"

class APawn;
class UMontageGraph;
class UMontageGraphNode;
class UHBActionComponent;

struct MONTAGEGRAPH_API FMontageGraphDelegates
{
	/** delegate type for combo graph execution events (Params: const UComboGraphAbilityTask_StartGraph*, const UComboGraph* ComboGraphAsset) */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMontageGraphEvaluated,
	                                     const UHBActionComponent& ActionComponent,
	                                     const UMontageGraphNode& Node);
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMontageGraphSelected,
	                                     const UMontageGraphNode& ActionComponent);


	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMontageGraphReset,
	                                     const UHBActionComponent& ActionComponent);


	/** Called when the Graph activates a node*/
	static FOnMontageGraphEvaluated OnGraphNodeEvaluated;

	
	/** Called when the Graph tries to find a node. */
	static FOnMontageGraphSelected OnGraphNodeSelected;

	/** Called when the Combo Graph ends execution. */
	static FOnMontageGraphReset OnGraphReset;


	/** delegate type for combo graph execution events (Params: const UComboGraphAbilityTask_StartGraph*, const UComboGraph* ComboGraphAsset) */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMontageGraphNodeEvaluated,
	                                     const UMontageGraphNode *Node);

	static FOnMontageGraphNodeEvaluated OnNodeEvaluated;

#if WITH_EDITORONLY_DATA

#endif
};
