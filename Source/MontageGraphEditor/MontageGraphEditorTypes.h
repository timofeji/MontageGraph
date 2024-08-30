
#pragma once

#include "CoreMinimal.h"
#include "MontageGraphEditorTypes.generated.h"

/** DataTable used to draw edge (transition) icons in Graph. Determine mappings between Keys and Icon textures */
USTRUCT(BlueprintType)
struct MONTAGEGRAPHEDITOR_API FMontageGraphInputsMetaData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage Graph")
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage Graph")
	TSoftObjectPtr<UTexture2D> Icon;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage Graph")
	// EHBMontageGraphIconType IconType;
};

/** Holds Graph Pin Categories Names as constants */
UCLASS()
class UMontageGraphPinNames : public UObject
{
	GENERATED_UCLASS_BODY()

	static const FName PinCategory_EntryOut;
	static const FName PinCategory_MultipleNodes;
	static const FName PinCategory_Transition;
	static const FName PinCategory_Edge;

	static const FName PinName_In;
	static const FName PinName_Out;
};


/** Predicate class for sorting operation in auto arrange */
struct FMontageGraphCompareNodeXLocation
{
	FORCEINLINE bool operator()(const UEdGraphPin& A, const UEdGraphPin& B) const
	{
		const UEdGraphNode* NodeA = A.GetOwningNode();
		const UEdGraphNode* NodeB = B.GetOwningNode();

		if (NodeA->NodePosX == NodeB->NodePosX)
		{
			return NodeA->NodePosY < NodeB->NodePosY;
		}

		return NodeA->NodePosX < NodeB->NodePosX;
	}
};

UENUM()
enum class EMontageGraphAutoArrangeStrategy : uint8
{
	/** Setup Event */
	Vertical,

	/** Update Event */
	Horizontal,

	/** None - invalid */
	None UMETA(Hidden),
};

