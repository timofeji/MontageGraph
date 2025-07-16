// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"

class FMontageGraphEditor;
class FExtender;
class FToolBarBuilder;

class FMontageGraphEditorToolbar : public TSharedFromThis<FMontageGraphEditorToolbar>
{
public:
	FMontageGraphEditorToolbar(TSharedPtr<FMontageGraphEditor> InMontageGraphEditor)
		: MGEditor(InMontageGraphEditor) {}

	void AddModesToolbar(TSharedPtr<FExtender> Extender);
	void AddDebuggerToolbar(TSharedPtr<FExtender> Extender);
	void AddGameplayToolbar(TSharedPtr<FExtender> Extender);

	void SetCreateActionsEnabled(bool bActionsEnabled);

private:
	void FillModesToolbar(FToolBarBuilder& ToolbarBuilder);
	void FillDebuggerToolbar(FToolBarBuilder& ToolbarBuilder);
	void FillGameplayToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the blueprint editor tool that owns us */
	TWeakPtr<FMontageGraphEditor> MGEditor;

private:
	bool bCreateActionsEnabled = true;
};
