#pragma once
#include "MontageGraphEditorSettings.generated.h"

class UDataTable;
class UInputMappingContext;

/** Not used right now but might be. Default behavior is render both */
UENUM()
enum class EMontageGraphDrawPinTypes : uint8
{
	/** Setup Event */
	Both UMETA(DisplayName="Draw both Vertical and Horizontal pins"),

	/** Update Event */
	Horizontal UMETA(DisplayName="Draw only Left and Right pins"),

	/** None - invalid */
	Vertical UMETA(DisplayName="Draw only Top and Bottom pins"),
};

UCLASS(Config = Game,
	defaultconfig)
class MONTAGEGRAPHEDITOR_API UMontageGraphEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UMontageGraphEditorSettings();

	/** Default background color for Montage nodes */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Colors")
	FLinearColor MontageNodeColor;

	/** Default background color for Sequence nodes */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Colors")
	FLinearColor SequenceNodeColor;

	/** Default background color for combo nodes in active states (during debug) */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Colors")
	FLinearColor DebugActiveColor;

	/**
	 * The duration used to interpolate the background color of nodes from Active to Default color when active states change (no longer active during debug).
	 *
	 * If set to 0.0, will disable color interpolation.
	 */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Debug",
		meta = (ClampMin = "0.0"))
	float DebugFadeTime;

	/** The padding around the main node box. */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Size & Paddings")
	FMargin ContentMargin;

	/** The padding within the main node box. */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Size & Paddings")
	FMargin ContentInternalPadding;

	/** The minimum desired sizes for pin connections. */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Size & Paddings",
		meta = (ClampMin = "20.0"))
	float PinSize = 20.f;

	/** The minimum amount of padding to draw around pin connections. */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Size & Paddings",
		meta = (ClampMin = "0.0"))
	float PinPadding = 20.f;

	// /** The minimum desired sizes for pin connections. */
	// UPROPERTY(config, EditAnywhere, Category = "Size & Paddings", meta = (ClampMin = "0.0"))
	// EHBMontageGraphDrawPinTypes DrawPinTypes = EHBMontageGraphDrawPinTypes::Both;

	// /** Enhanced Input Context Mapping to use to draw edge (transition) icons in Graphs */
	// UPROPERTY(config, EditAnywhere, Category = "Icons")
	// TSoftObjectPtr<UInputMappingContext> ContextMapping;

	/**
	 * Path to the DataTable used to draw edge (transition) icons in Graph. Determine mappings between Keys and Icon textures.
	 *
	 * This is set by default to an internal DataTables (that you can find in `/HBMontageGraph/Xelu_Icons/`) that setup texture icons for every keyboard and gamepad key.
	 *
	 * Icons are coming from Xelu's Free Controllers & Keyboard Prompts: https://thoseawesomeguys.com/prompts/
	 * Thanks to "Nicolae (Xelu) Berbece" and "Those Awesome Guys" to make it available in the public domain licence under Creative Commons 0 (CC0)
	 */
	// UPROPERTY(config, EditDefaultsOnly, Category = "Icons", meta = (RequiredAssetDataTags = "RowStructure=HBMontageGraphInputsMetaData"))
	// TSoftObjectPtr<UDataTable> IconsDataTable;
	//
	// /** Icon preference to draw edge (transition) icons in Graph. Can be either Keyboard or Gamepad based */
	// UPROPERTY(config, EditDefaultsOnly, Category = "Icons")
	// EHBMontageGraphIconType IconPreference = EHBMontageGraphIconType::Keyboard;

	/** Size of Icons when drawing edges (transitions) in HBMontageGraphs */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Icons",
		meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float IconSize;

	/** The Slot Name to use with dynamic montages, created from sequences */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Auto Setup")
	FName DynamicMontageSlotName;

	// /** Map of Auto Setup Animation Notify States. The key is the Anim Notify State class, the value is the time start / end definition in percent */
	// UPROPERTY(config, EditAnywhere, Category = "Auto Setup")
	// TMap<TSoftClassPtr<UAnimNotifyState>, FHBMontageGraphNotifyStateAutoSetup> NotifyStates;
	//
	// /** Flag to enable / disable message warnings (logs and on screen) about Sequences being used in a networked environment */
	UPROPERTY(config,
		EditAnywhere,
		Category = "Warnings",
		AdvancedDisplay)
	bool bSequencesNetworkedWarning = true;

	/** Flag to disable validation warnings on startup (about missing config for AbilitySystemGlobals). Config writable but not exposed in editor. */
	UPROPERTY(config)
	bool bIgnoreStartupValidationWarnings = false;
};
