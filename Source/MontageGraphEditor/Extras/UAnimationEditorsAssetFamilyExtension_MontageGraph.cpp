// Copyright Drop Games Inc.
#include "UAnimationEditorsAssetFamilyExtension_MontageGraph.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "MontageGraph/MontageGraph.h"
#include "PhysicsEngine/PhysicsAsset.h"


#define LOCTEXT_NAMESPACE "UAnimationEditorsAssetFamilyExtension_MontageGraph"
// UAnimationEditorsAssetFamilyExtension_MontageGraphAsset

TObjectPtr<UClass> UAnimationEditorsAssetFamilyExtension_MontageGraphAsset::GetAssetClass() const
{
	return UMontageGraph::StaticClass();
}

FText UAnimationEditorsAssetFamilyExtension_MontageGraphAsset::GetAssetTypeDisplayName() const
{
	return LOCTEXT("MontageGraphAssetDisplayName", "MontageGraph");
}

const FSlateBrush* UAnimationEditorsAssetFamilyExtension_MontageGraphAsset::GetAssetTypeDisplayIcon() const
{
	return FAppStyle::Get().GetBrush("Persona.AssetClass.SkeletalMesh");
}

void UAnimationEditorsAssetFamilyExtension_MontageGraphAsset::FindAssetsOfType(TArray<FAssetData>& OutAssets, const IAnimationEditorsAssetFamilyInterface& AssetFamilyInterface) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.ClassPaths.Add(UMontageGraph::StaticClass()->GetClassPathName());

	// If we have a mesh, look for a physics asset that has that mesh as its preview mesh
	if (TObjectPtr<const USkeletalMesh> SkeletalMeshAsset = AssetFamilyInterface.GetAssetOfType<USkeletalMesh>())
	{
		Filter.TagsAndValues.Add(GET_MEMBER_NAME_CHECKED(UMontageGraph, PreviewSkeletalMesh), FSoftObjectPath(SkeletalMeshAsset).ToString());
	}

	AssetRegistryModule.Get().GetAssets(Filter, OutAssets);

	// // If we have a mesh and it has a physics asset, use it but only if its different from the one on the preview mesh
	// if (TObjectPtr<const USkeletalMesh> SkeletalMeshAsset = AssetFamilyInterface.GetAssetOfType<USkeletalMesh>())
	// {
	// 	if (UPhysicsAsset* MeshPhysicsAsset = SkeletalMeshAsset->GetPhysicsAsset())
	// 	{
	// 		OutAssets.AddUnique(FAssetData(MeshPhysicsAsset));
	// 	}
	// }
}
bool UAnimationEditorsAssetFamilyExtension_MontageGraphAsset::IsAssetCompatible(const FAssetData& InAssetData, const IAnimationEditorsAssetFamilyInterface& AssetFamilyInterface) const
{
	// If our mesh is valid and this is the physics asset used on it, we are compatible
	TObjectPtr<const USkeletalMesh> SkeletalMeshAsset = AssetFamilyInterface.GetAssetOfType<USkeletalMesh>();
	FAssetDataTagMapSharedView::FFindTagResult Result = InAssetData.TagsAndValues.FindTag(GET_MEMBER_NAME_CHECKED(UMontageGraph, PreviewSkeletalMesh));
	if (Result.IsSet() && SkeletalMeshAsset)
	{
		return Result.GetValue() == FSoftObjectPath(SkeletalMeshAsset).ToString();
	}

	return false;
}

void UAnimationEditorsAssetFamilyExtension_MontageGraphAsset::FindCounterpartAssets(const UObject* InAsset, IAnimationEditorsAssetFamilyInterface& AssetFamilyInterface)
{
	const UMontageGraph* MontageGraph = CastChecked<const UMontageGraph>(InAsset);

	TObjectPtr<USkeletalMesh> SkeletalMesh = MontageGraph->PreviewSkeletalMesh.LoadSynchronous();
	if (SkeletalMesh)
	{
		AssetFamilyInterface.SetAssetOfType<USkeletalMesh>(SkeletalMesh);

		AssetFamilyInterface.SetAssetOfType<USkeleton>(SkeletalMesh->GetSkeleton());
	}
};

void UAnimationEditorsAssetFamilyExtension_MontageGraphAsset::GetPosition(FName& OutBeforeClass, FName& OutAfterClass) const
{
	OutBeforeClass = NAME_None;
	OutAfterClass = UPhysicsAsset::StaticClass()->GetFName();
}
#undef  LOCTEXT_NAMESPACE
