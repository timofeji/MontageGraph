#pragma once
#include "AssetToolsModule.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "MontageGraph/MontageGraph.h"
#include "Input/Reply.h"
#include "PersonaDelegates.h"

class IPropertyHandle;
class USkeleton;
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

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static void SortCustomDetailsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap);
};
