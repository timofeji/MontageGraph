#include "MontageGraphDetails.h"

#include <Graph/EdNodes/MGEdNode_Montage.h>
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "PersonaTabs.h"
#include "Graph/MontageGraphEdGraph.h"
#include "MontageGraph/Nodes/MGNode_Montage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FMontageGraphDetails"

// FScopedTransaction Transaction(FText::FromString(NewKeys[0]));
// Modify();
// GetGraph()->NotifyNodeChanged(this);
void FMontageGraphDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// DetailBuilder.SortCategories(&SortCustomDetailsCategories);
}

void FMontageGraphDetails ::SortCustomDetailsCategories(const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap)
{
	// (*AllCategoryMap.Find(FName("Editor")))->SetSortOrder(1000);
}



#undef LOCTEXT_NAMESPACE
