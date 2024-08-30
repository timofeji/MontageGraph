// Copyright Timofej Jermolaev, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


struct FMontageGraphSequencerExtensions;

class FMontageGraphEditorModule : public IModuleInterface
{
public:
	void ExtendSequencerToolbar(FToolBarBuilder& ToolBarBuilder) const;
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	
	TSharedPtr<FGraphPanelNodeFactory> GraphNodeFactory;

	TSharedPtr<FMontageGraphSequencerExtensions> SequencerExtensions;
};
