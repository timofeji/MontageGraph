#pragma once
#include "IDetailCustomization.h"

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
};
