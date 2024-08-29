#pragma once
#include "AssetToolsModule.h"
#include "IDetailCustomization.h"
#include "MontageGraph/MontageGraph.h"

class ILevelSequenceEditorToolkit;
class ISequencer;
class UAnimSeqExportOption;
class UMontageGraphEdNodeMontage;
class UControlRig;
/**
 * 
 */
class MONTAGEGRAPHEDITOR_API FMontageGraphDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FMontageGraphDetails);
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	bool CreateAnimationSequence(const TArray<UObject*> NewAssets, USkeletalMeshComponent* SkelMeshComp, FGuid Binding, bool bCreateSoftLink, TSharedPtr<ISequencer> SequencerPtr) const;
	void CreateLinkedAnimationAssets(UMontageGraphEdNodeMontage* AnimNode, FAssetToolsModule& AssetToolsModule,
	                            USkeletalMesh* SkeletalMesh) const;
	void CreateLinkedControlRigAnimationForNode(UMontageGraphEdNodeMontage* AnimNode) const;
	void OpenLinkedAnimation(UMontageGraphEdNodeMontage* MontageEdNode);

	UMontageGraph* GraphBeingEdited;

private:
	TObjectPtr<UAnimSeqExportOption> AnimSeqExportOption;

};
