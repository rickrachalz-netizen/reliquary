// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RLTypes.h"
#include "RLRecipe.generated.h"

/** One material requirement in a recipe. Accepts either a specific material or a whole family. */
USTRUCT(BlueprintType)
struct FRLRecipeIngredient
{
    GENERATED_BODY()

    /** If set, requires this exact material id. Takes priority over Family. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
    FName MaterialId = NAME_None;

    /** If MaterialId is None, any material of this family satisfies the requirement. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
    ERLMaterialFamily Family = ERLMaterialFamily::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient", meta = (ClampMin = 1))
    int32 Count = 1;
};

/**
 *  A crafting recipe: consume ingredients at the base-camp forge to produce a
 *  gear item. The specific materials used feed into the resulting item's power
 *  and flavour (see URLCraftingSubsystem).
 */
UCLASS(BlueprintType)
class RELIQUARY_API URLRecipe : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FName RecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    /** GearId of the template produced. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
    FName OutputGearId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TArray<FRLRecipeIngredient> Ingredients;

    /** Character level required to craft this recipe. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    int32 RequiredLevel = 1;

    /** Recipes known from the start are always craftable; others must be discovered. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
    bool bKnownByDefault = false;
};
