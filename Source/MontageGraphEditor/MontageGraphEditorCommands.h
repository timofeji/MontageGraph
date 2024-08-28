// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
/**
 * 
 */

class FMontageGraphEditorCommands  : public TCommands<FMontageGraphEditorCommands>
{
public:
	FMontageGraphEditorCommands();

	TSharedPtr<FUICommandInfo> AutoArrange;
	TSharedPtr<FUICommandInfo> AutoArrangeVertical;
	TSharedPtr<FUICommandInfo> AutoArrangeHorizontal;
	TSharedPtr<FUICommandInfo> EditClassDefaults;


	// TCommands<> interface
	virtual void RegisterCommands() override;
	// End of TCommands<> interface
};
