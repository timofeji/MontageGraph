#pragma once
#include "CoreMinimal.h"
#include "DopeSheet/SDopeSheetTrackSection.h"
#include "DopeSheet/Tracks/DopeSheetTrackBase.h"
#include "DopeSheet/Tracks/SDopeSheetTrackTimeline.h"
#include "MontageTrack_Sequences.generated.h"

class FDopeSheetController;

#if WITH_EDITOR
UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrackSection_Sequences : public UDopeSheetTrackSection
{
	GENERATED_BODY()

	
public:

	UPROPERTY(EditAnywhere, Category = "Settings", meta=(DisplayPriority=1))
	UAnimSequence* AnimSequence;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName SlotName = FName("DefaultSlot");
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName SectionName = FName("Default");

	UPROPERTY(EditAnywhere, Category = "Blend")
	FMontageBlendSettings BlendInSettings;

	UPROPERTY(EditAnywhere, Category = "Blend")
	FMontageBlendSettings BlendOutSettings;
};


class SMGSequenceTrack;
class SMGSequenceTrackSection;

/** Which edge of a sequence section a blend handle represents. */
enum class EMGBlendHandleType : uint8
{
	BlendIn,
	BlendOut,
};

/** Drag-drop operation that modifies blend-in or blend-out time on a sequence section. */
struct FMGBlendHandleDragDrop : public FDragDropOperation
{
	DRAG_DROP_OPERATOR_TYPE(FMGBlendHandleDragDrop, FDragDropOperation);

	virtual void Construct() override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;
	virtual void OnDragged(const FDragDropEvent& DragDropEvent) override;
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	static TSharedRef<FMGBlendHandleDragDrop> New(
		TWeakObjectPtr<UMontageTrackSection_Sequences> InSectionData,
		TSharedPtr<FDopeSheetController>               InController,
		EMGBlendHandleType                             InHandleType);

	TWeakObjectPtr<UMontageTrackSection_Sequences> SectionDataPtr;
	TSharedPtr<FDopeSheetController>               ControllerPtr;
	EMGBlendHandleType                             HandleType;
};

/** Small draggable handle widget placed at the left/right edge of a sequence section. */
class SMGSequenceBlendHandle : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMGSequenceBlendHandle) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UMontageTrackSection_Sequences>, SectionData)
		SLATE_ARGUMENT(TSharedPtr<FDopeSheetController>, Controller)
		SLATE_ARGUMENT(EMGBlendHandleType, HandleType)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void   OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void   OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	TWeakObjectPtr<UMontageTrackSection_Sequences> SectionDataPtr;
	TSharedPtr<FDopeSheetController>               ControllerPtr;
	EMGBlendHandleType                             HandleType;
	bool                                           bIsHovered = false;
};

/** Implements drag and drop operation. */
struct FDopeSheetSequenceSectionDragDrop : public FDragDropOperation
{
	DRAG_DROP_OPERATOR_TYPE(FDopeSheetSectionDragDrop, FDragDropOperation);

	virtual void Construct() override;

	virtual void OnDrop( bool bDropWasHandled, const FPointerEvent& MouseEvent ) override;

	virtual void OnDragged( const class FDragDropEvent& DragDropEvent ) override;
	

	static TSharedRef<FDopeSheetSequenceSectionDragDrop> New(
		TSharedRef<SMGSequenceTrack> ParentTrack,
		TSharedRef<SWidget> SectionWidget,
		const FVector2D&    CursorPosition,
		const FVector2D&    ScreenPositionOfNode);

	/** Gets the widget that will serve as the decorator unless overridden. If you do not override, you will have no decorator */
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	friend class SDopeSheetTrack;
	friend class SDopeSheetTrackSection;

protected:
	TWeakPtr<SMGSequenceTrack> OwningTrack;
	TWeakPtr<SWidget>          SectionBeingDragged;
	FVector2D                  Offset;
	FVector2D                  StartingScreenPos;
	
};


class SMGSequenceTrackSection : public SDopeSheetTrackSection
{
public:
	SLATE_BEGIN_ARGS(SMGSequenceTrackSection)
		{
		}

		SLATE_ARGUMENT(UAnimSequence*, Sequence)
		SLATE_ARGUMENT(UMontageTrackSection_Sequences*, SectionData)
		SLATE_ARGUMENT(TSharedPtr<FDopeSheetController>, Controller)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 InLayerId,
	                      const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void   OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void   OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	void           Select();

protected:
	UAnimSequence*                                 SequencePtr;
	TWeakObjectPtr<UMontageTrackSection_Sequences> SectionDataPtr;
	TSharedPtr<FDopeSheetController>               ControllerPtr;
};


class SMGSequenceTrack : public SDopeSheetTrackTimeline
{
public:
	SLATE_BEGIN_ARGS(SMGSequenceTrack)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const TSharedPtr<FDopeSheetTrackViewModel>& TrackModelPtr);
	~SMGSequenceTrack();


	void         RegenerateSequenceSections();
	void         OnRearrangeDrop();
	void         PreviewRearrange(FVector2D ScreenCursorPos);
	virtual void MakeSectionContextMenu(FMenuBuilder& ContextMenuBuilder) override;


	virtual void
	OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual bool   PassesAssetReferenceFilter(const TArray<FAssetData>& Array) override;
	virtual void   OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void   OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override;


protected:
	bool bIsValidAssetDrop        = false;
	bool bPreviewSectionRearrange = false;

	// TSharedRef<SWidget>  SelectedSectionWidget;
	TSharedRef<SWidget> SelectedSectionWidget = SNullWidget::NullWidget;
	int32               SelectedSectionIndex;
	TArray<int32>       RearrangeIndices;
};

UCLASS()
class MONTAGEGRAPHEDITOR_API UMontageTrack_Sequences : public UDopeSheetTrackBase
{
public:
	GENERATED_BODY()

	UMontageTrack_Sequences(const FObjectInitializer& ObjectInitializer);
	
	virtual UObject* GenerateNewDataAsset(UObject* Outer, FName Name) override;

	virtual void BakeToNode(UMGNode_Montage* RuntimeNode, FMGBakedNodeData& BakedData,
	                        UMontageGraph* OwnerGraph, const FString& DisplayName) override;

	virtual bool CanCreateSubTracks() override;
	
	virtual void GetSubTrackClasses(TArray<UClass*>& TrackClasses) override;
	
	/*Begin IDopeSheetTrack Interface*/
	virtual TSharedRef<SWidget> MakeTrackTimelineWidget(TSharedPtr<FDopeSheetTrackViewModel> Shared) const override;
	virtual void                DroppedAssetsOnTrack(TArray<FAssetData> AssetArr) override;
	
	virtual const FText         GetTrackName() const override;
	virtual const FLinearColor  GetTrackColor() const override;
	virtual FSlateIcon          GetTrackIcon() const override;
	/*End IDopeSheetTrack Interface*/

};


#endif