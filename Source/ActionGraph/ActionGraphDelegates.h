#pragma once

#include "CoreMinimal.h"

class APawn;
class UActionGraph;
class UActionGraphNode;
class UHBActionComponent;

struct ACTIONGRAPH_API FActionGraphDelegates
{
	/** delegate type for combo graph execution events (Params: const UComboGraphAbilityTask_StartGraph*, const UComboGraph* ComboGraphAsset) */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActionGraphEvaluated,
	                                     const UHBActionComponent& ActionComponent,
	                                     const UActionGraphNode& Node);
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnActionGraphSelected,
	                                     const UActionGraphNode& ActionComponent);


	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnActionGraphReset,
	                                     const UHBActionComponent& ActionComponent);


	/** Called when the Graph activates a node*/
	static FOnActionGraphEvaluated OnGraphNodeEvaluated;

	
	/** Called when the Graph tries to find a node. */
	static FOnActionGraphSelected OnGraphNodeSelected;

	/** Called when the Combo Graph ends execution. */
	static FOnActionGraphReset OnGraphReset;


	/** delegate type for combo graph execution events (Params: const UComboGraphAbilityTask_StartGraph*, const UComboGraph* ComboGraphAsset) */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnActionGraphNodeEvaluated,
	                                     const UActionGraphNode *Node);

	static FOnActionGraphNodeEvaluated OnNodeEvaluated;

#if WITH_EDITORONLY_DATA

#endif
};
