#pragma once

#include "CoreMinimal.h"

class APawn;
class UMontageGraph;
class UMGNode;
class UMontageGraphComponent;

struct MONTAGEGRAPH_API FMontageGraphDelegates
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMontageGraphProcessed,
	                                     const UMontageGraphComponent* Component,
	                                     const UMGNode& Node);
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMontageGraphInitialized,
	                                     const UMontageGraphComponent* Component);
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMontageGraphReset,
	                                     const UMontageGraphComponent& Component);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMontageGraphNodeEvaluated,
	                                     const UMGNode *Node);

	/** Called when the Graph activates a node*/
	static FOnMontageGraphProcessed OnGraphNodeLinked;

	
	/** Called when the Graph tries to find a node. */
	static FOnMontageGraphProcessed OnGraphNodeSelected;
	
	/** Called when the current node is first initialized. */
	static FOnMontageGraphInitialized OnGraphInitialized;

	/** Called when the current node is reset. */
	static FOnMontageGraphReset OnGraphReset;



	static FOnMontageGraphNodeEvaluated OnNodeEvaluated;

#if WITH_EDITORONLY_DATA

#endif
};
