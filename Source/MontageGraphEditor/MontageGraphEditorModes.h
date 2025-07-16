
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "MontageGraphEditor.h"
#include "BlueprintEditorModes.h"

/** Application mode for gameplay editing` */
class FMontageGraphEditorMode_Selection : public FApplicationMode
{
public:
	FMontageGraphEditorMode_Selection (const TSharedRef<FMontageGraphEditor>& InMontageGraphEditor);

	virtual void RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) override;
	virtual void PreDeactivateMode() override;
	virtual void PostActivateMode() override;

protected:
	TWeakPtr<FMontageGraphEditor> MGEditor;

	// Set of spawnable tabs in gameplay mode
	FWorkflowAllowedTabSet TabFactories;
};

/** Application mode for editing node/edge blends */
class FMontageGraphEditorMode_Blends : public FApplicationMode
{
public:
	FMontageGraphEditorMode_Blends(const TSharedRef<FMontageGraphEditor>& InMontageGraphEditor);

	virtual void RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) override;
	
	virtual void PostActivateMode() override;
	

protected:
	TWeakPtr<FMontageGraphEditor> MGEditor;

	// Set of spawnable tabs in blends mode
	FWorkflowAllowedTabSet TabFactories;
};



/** Application mode for editing node/edge blends */
class FMontageGraphEditorMode_Debug: public FApplicationMode
{
public:
	FMontageGraphEditorMode_Debug(const TSharedRef<FMontageGraphEditor>& InMontageGraphEditor);

	virtual void RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) override;

protected:
	TWeakPtr<FMontageGraphEditor> MGEditor;
	
	// Set of spawnable tabs in blackboard mode

	FWorkflowAllowedTabSet TabFactories;
};
