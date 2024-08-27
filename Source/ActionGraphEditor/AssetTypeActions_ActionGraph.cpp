// Copyright Drop Games Inc. 

#include "AssetTypeActions_ActionGraph.h"

#include "ActionGraphEditor.h"
#include "ActionGraphEditorLog.h"


void FAssetTypeActions_ActionGraph::OpenAssetEditor(const TArray<UObject*>& InObjects,
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
		if (UActionGraph* Character = Cast<UActionGraph>(Object))
		{
			ACTIONGRAPH_LOG(
				Warning,
				TEXT("Tried to create a Character = `%s`"),
				*Character->GetPathName());

			const TSharedRef<FActionGraphEditor> NewCharacterEditor(new
				FActionGraphEditor());
			NewCharacterEditor->InitCharacterEditor(Mode,
			                                        EditWithinLevelEditor,
			                                        Character);
		}
	}
}
