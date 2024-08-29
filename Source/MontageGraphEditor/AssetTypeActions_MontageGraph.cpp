// Copyright Drop Games Inc. 

#include "AssetTypeActions_MontageGraph.h"

#include "MontageGraphEditor.h"
#include "MontageGraphEditorLog.h"
#include "SSkeletonWidget.h"

#define LOCTEXT_NAMESPACE "MontageGraphAsset"

bool FAssetTypeActions_MontageGraph::ReplaceMissingSkeleton(TArray<TObjectPtr<UObject>> Array) const
{
	// record anim assets that need skeleton replaced
	const TArray<TWeakObjectPtr<UObject>> AnimsToFix = GetTypedWeakObjectPtrs<UObject>(Array);
	// get a skeleton from the user and replace it
	const TSharedPtr<SReplaceMissingSkeletonDialog> PickSkeletonWindow = SNew(SReplaceMissingSkeletonDialog).
		AnimAssets(AnimsToFix);
	const bool bWasSkeletonReplaced = PickSkeletonWindow.Get()->ShowModal();
	return bWasSkeletonReplaced;
}

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
		if (UMontageGraph* GraphToEdit = Cast<UMontageGraph>(Object))
		{
			// USkeleton* AnimSkeleton = GraphToEdit->GetSkeleton();
			// if (!AnimSkeleton)
			// {
			// 	FText ShouldRetargetMessage = LOCTEXT("ShouldRetargetAnimAsset_Message",
			// 	                                      "Could not find the skeleton for Montage Graph '{GraphName}' Would you like to choose a new one?");
			//
			// 	FFormatNamedArguments Arguments;
			// 	Arguments.Add(TEXT("GraphName"), FText::FromString(GraphToEdit->GetName()));
			//
			// 	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(ShouldRetargetMessage, Arguments)) ==
			// 		EAppReturnType::Yes)
			// 	{
			// 		TArray<TObjectPtr<UObject>> AssetsToRetarget;
			// 		AssetsToRetarget.Add(GraphToEdit);
			// 		const bool bSkeletonReplaced = ReplaceMissingSkeleton(AssetsToRetarget);
			// 		if (!bSkeletonReplaced)
			// 		{
			// 			return; // Persona will crash if trying to load asset without a skeleton
			// 		}
			// 	}
			// 	else
			// 	{
			// 		return;
			// 	}
			// }
			//

			MG_ERROR(
				Warning,
				TEXT("Tried to create a Character = `%s`"),
				*GraphToEdit->GetPathName());

			const TSharedRef<FMontageGraphEditor> NewCharacterEditor(new
				FMontageGraphEditor());
			NewCharacterEditor->InitCharacterEditor(Mode,
			                                        EditWithinLevelEditor,
			                                        GraphToEdit);
		}
	}
}
#undef LOCTEXT_NAMESPACE
