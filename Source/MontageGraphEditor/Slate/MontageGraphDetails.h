#pragma once
#include "IDetailCustomization.h"
#include "MontageGraph/MontageGraph.h"

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
	
	void CreateLinkedControlRigMontageForNode(UMontageGraphEdNodeMontage* AnimNode) const;

	UMontageGraph* GraphBeingEdited;
};
