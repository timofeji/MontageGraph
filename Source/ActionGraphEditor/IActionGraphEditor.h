


#pragma once

#include "PersonaAssetEditorToolkit.h"
#include "IHasPersonaToolkit.h"

class IActionGraphEditor:
	public FPersonaAssetEditorToolkit,
	public IHasPersonaToolkit
//	, public IHasMenuExtensibility
//	, public IHasToolBarExtensibility
{
public:
	virtual ~IActionGraphEditor() override = default;
};
