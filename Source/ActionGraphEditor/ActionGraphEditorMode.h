
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "ActionGraphEditor.h"
#include "BlueprintEditorModes.h"


class FActionGraphEditorMode : public FApplicationMode
{
public:
	FActionGraphEditorMode(const TSharedPtr<FActionGraphEditor>& InHBCharacterEditor);

	// FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;

protected:
	// Set of spawnable tabs
	FWorkflowAllowedTabSet TabFactories;
	TWeakPtr<FActionGraphEditor> CharacterBlueprint;
};
