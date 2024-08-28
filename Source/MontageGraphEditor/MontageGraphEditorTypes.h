
#pragma once

#include "CoreMinimal.h"
#include "MontageGraphEditorTypes.generated.h"

/** DataTable used to draw edge (transition) icons in Graph. Determine mappings between Keys and Icon textures */
USTRUCT(BlueprintType)
struct MONTAGEGRAPHEDITOR_API FHBMontageGraphInputsMetaData : public FTableRowBase
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
struct FHBMontageGraphCompareNodeXLocation
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
enum class EHBMontageGraphAutoArrangeStrategy : uint8
{
	/** Setup Event */
	Vertical,

	/** Update Event */
	Horizontal,

	/** None - invalid */
	None UMETA(Hidden),
};

struct FHBMontageGraphEditorModes
{
	// Mode constants
	static const FName HBMontageGraphPersonaModeID;
	static const FName HBMontageGraphDefaultModeID;

	static FText GetLocalizedMode(const FName InMode)
	{
		static TMap<FName, FText> LocModes;

		if (LocModes.Num() == 0)
		{
			// LocModes.Add(HBMontageGraphPersonaModeID, NSLOCTEXT("HBMontageGraphEditorModes", "HBMontageGraphPersonaMode", "Persona Preview"));
			// LocModes.Add(HBMontageGraphDefaultModeID, NSLOCTEXT("HBMontageGraphEditorModes", "HBMontageGraphDefaultMode", "Montage Graph"));
		}

		check(InMode != NAME_None);
		const FText* OutDesc = LocModes.Find(InMode);
		check(OutDesc);
		return *OutDesc;
	}
private:
	FHBMontageGraphEditorModes() {}
};

