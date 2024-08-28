
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "MontageGraphEditor.h"
#include "BlueprintEditorModes.h"


class FMontageGraphEditorMode : public FApplicationMode
{
public:
	FMontageGraphEditorMode(const TSharedPtr<FMontageGraphEditor>& InHBCharacterEditor);

	// FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;

protected:
	// Set of spawnable tabs
	FWorkflowAllowedTabSet TabFactories;
	TWeakPtr<FMontageGraphEditor> CharacterBlueprint;
};
