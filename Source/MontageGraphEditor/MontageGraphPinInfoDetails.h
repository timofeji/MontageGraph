// Created by Timofej Jermolaev, All rights reserved . 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MontageGraphPinInfoDetails.generated.h"

/** Required info for reconstructing a manually specified pin */
USTRUCT()
struct FMontageGraphPinInfo
{
	GENERATED_BODY()

	FMontageGraphPinInfo()
		: Name(NAME_None)
	{
		Type.ResetToDefaults();
	}

	FMontageGraphPinInfo(const FName& InName, const FEdGraphPinType& InType)
		: Name(InName)
		, Type(InType)
	{
	}

	/** The name of this parameter */
	UPROPERTY(EditAnywhere, Category = "Inputs")
	FName Name;

	/** The type of this parameter */
	UPROPERTY(EditAnywhere, Category = "Inputs")
	FEdGraphPinType Type;
};
/**
 * 
 */
class FMontageGraphPinInfoDetails : public IPropertyTypeCustomization
{
 
public:
	/** The name of this parameter */
	UPROPERTY(EditAnywhere, Category = "Inputs")
	FName Name;

	/** The type of this parameter */
	UPROPERTY(EditAnywhere, Category = "Inputs")
	FEdGraphPinType Type;

	
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FMontageGraphPinInfoDetails());
	}

	// IDetailCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InChildBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) {};

private:
	/** Check to see if this name is valid */
	bool IsNameValid(const FString& InNewName);

	/** UI handlers */
	FEdGraphPinType GetTargetPinType() const;
	void HandlePinTypeChanged(const FEdGraphPinType& InPinType);
	void HandleTextChanged(const FText& InNewText);
	void HandleTextCommitted(const FText& InNewText, ETextCommit::Type InCommitType);

private:
	/** Property handles to edit */
	TSharedPtr<IPropertyHandle> NamePropertyHandle;
	TSharedPtr<IPropertyHandle> TypePropertyHandle;
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	/** Text box for editing names */
	TSharedPtr<SEditableTextBox> NameTextBox;

	/** The node we are editing on */
	// TWeakObjectPtr<UAnimGraphNode_LinkedInputPose> WeakOuterNode;
};
