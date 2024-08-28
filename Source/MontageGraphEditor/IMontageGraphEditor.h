


#pragma once

#include "PersonaAssetEditorToolkit.h"
#include "IHasPersonaToolkit.h"

class IMontageGraphEditor:
	public FPersonaAssetEditorToolkit,
	public IHasPersonaToolkit
//	, public IHasMenuExtensibility
//	, public IHasToolBarExtensibility
{
public:
	virtual ~IMontageGraphEditor() override = default;
};
