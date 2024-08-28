// Copyright Drop Games Inc. 

#include "AssetTypeActions_MontageGraph.h"

#include "MontageGraphEditor.h"
#include "MontageGraphEditorLog.h"


void FAssetTypeActions_MontageGraph::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                                    TSharedPtr<IToolkitHost>
                                                    EditWithinLevelEditor)
{
	// FAssetTypeActions_Base::OpenAssetEditor(InObjects,
	//                                         EditWithinLevelEditor);

	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		                                ? EToolkitMode::WorldCentric
		                                : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		// Only handle dialogues
		if (UMontageGraph* Character = Cast<UMontageGraph>(Object))
		{
			MontageGraph_LOG(
				Warning,
				TEXT("Tried to create a Character = `%s`"),
				*Character->GetPathName());

			const TSharedRef<FMontageGraphEditor> NewCharacterEditor(new
				FMontageGraphEditor());
			NewCharacterEditor->InitCharacterEditor(Mode,
			                                        EditWithinLevelEditor,
			                                        Character);
		}
	}
}
