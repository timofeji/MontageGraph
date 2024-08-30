#pragma once
#include "CoreMinimal.h"

struct FMontageGraphSequencerExtensions : public TSharedFromThis<FMontageGraphSequencerExtensions>
{
	void ExtendSequencerToolbar(FToolBarBuilder& ToolbarBuilder) const;
	void AddSharedPose() const;
	bool CanAddSharedPose() const;
	void Register() const;
	void Unregister() const;
};

class FMontageGraphSequencerCommands : public TCommands<FMontageGraphSequencerCommands>
{
public:
	FMontageGraphSequencerCommands()
		: TCommands<FMontageGraphSequencerCommands>
		(
			TEXT("MontageGraphSequncerCommands"),
			NSLOCTEXT("Contexts", "MontageGraphSequncerCommands", "Anim Sequence Montage Graph extensions"),
			NAME_None,
			FAppStyle::GetAppStyleSetName()
		)
	{
	}


	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> AddSharedPose;
};
