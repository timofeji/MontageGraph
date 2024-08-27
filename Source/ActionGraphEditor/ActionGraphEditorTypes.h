
#pragma once

#include "CoreMinimal.h"
#include "ActionGraphEditorTypes.generated.h"

/** DataTable used to draw edge (transition) icons in Graph. Determine mappings between Keys and Icon textures */
USTRUCT(BlueprintType)
struct ACTIONGRAPHEDITOR_API FHBActionGraphInputsMetaData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HBAction Graph")
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HBAction Graph")
	TSoftObjectPtr<UTexture2D> Icon;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HBAction Graph")
	// EHBActionGraphIconType IconType;
};

/** Holds Graph Pin Categories Names as constants */
UCLASS()
class UActionGraphPinNames : public UObject
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
struct FHBActionGraphCompareNodeXLocation
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
enum class EHBActionGraphAutoArrangeStrategy : uint8
{
	/** Setup Event */
	Vertical,

	/** Update Event */
	Horizontal,

	/** None - invalid */
	None UMETA(Hidden),
};

struct FHBActionGraphEditorModes
{
	// Mode constants
	static const FName HBActionGraphPersonaModeID;
	static const FName HBActionGraphDefaultModeID;

	static FText GetLocalizedMode(const FName InMode)
	{
		static TMap<FName, FText> LocModes;

		if (LocModes.Num() == 0)
		{
			// LocModes.Add(HBActionGraphPersonaModeID, NSLOCTEXT("HBActionGraphEditorModes", "HBActionGraphPersonaMode", "Persona Preview"));
			// LocModes.Add(HBActionGraphDefaultModeID, NSLOCTEXT("HBActionGraphEditorModes", "HBActionGraphDefaultMode", "HBAction Graph"));
		}

		check(InMode != NAME_None);
		const FText* OutDesc = LocModes.Find(InMode);
		check(OutDesc);
		return *OutDesc;
	}
private:
	FHBActionGraphEditorModes() {}
};

