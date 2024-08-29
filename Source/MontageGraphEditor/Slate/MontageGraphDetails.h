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

	void CreateLinkedControlRigAnimationForNode(UMontageGraphEdNodeMontage* AnimNode) const;

	UMontageGraph* GraphBeingEdited;
private:
	TObjectPtr<UAnimSeqExportOption> AnimSeqExportOption;

};
