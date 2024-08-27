// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
/**
 * 
 */

class FActionGraphEditorCommands  : public TCommands<FActionGraphEditorCommands>
{
public:
	FActionGraphEditorCommands();

	TSharedPtr<FUICommandInfo> AutoArrange;
	TSharedPtr<FUICommandInfo> AutoArrangeVertical;
	TSharedPtr<FUICommandInfo> AutoArrangeHorizontal;
	TSharedPtr<FUICommandInfo> RegenerateActionTracers;


	// TCommands<> interface
	virtual void RegisterCommands() override;
	// End of TCommands<> interface
};
