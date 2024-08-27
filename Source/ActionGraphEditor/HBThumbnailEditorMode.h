
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "ActionGraphEditor.h"
#include "BlueprintEditorModes.h"

class UHBCharacterBlueprint;

class FHBThumbnailEditorMode : public FApplicationMode
{
public:
	FHBThumbnailEditorMode(const TSharedRef<FActionGraphEditor>& InHBCharacterEditor);

	// FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;

protected:
	// Set of spawnable tabs
	FWorkflowAllowedTabSet TabFactories;

private:
	TWeakObjectPtr<UHBCharacterBlueprint> HBCharacterBlueprintPtr;
};
