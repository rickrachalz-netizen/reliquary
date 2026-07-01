// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RLTypes.h"
#include "RLGearDefinition.h"
#include "RLCraftingSubsystem.generated.h"

class URLInventoryComponent;
class URLRecipe;
class URLItemRegistry;

/** Result of a craft attempt, surfaced to UI. */
USTRUCT(BlueprintType)
struct FRLCraftResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Craft")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Craft")
    FText FailureReason;

    UPROPERTY(BlueprintReadOnly, Category = "Craft")
    FRLGearInstance CraftedItem;
};

/**
 *  Crafting — the heart of the game. Turns gathered materials into permanent
 *  gear. The specific materials chosen feed the item's power and rarity, so a
 *  finer material makes a better sword. Gear starts modest but ramps hard via
 *  rolled affixes on top of the template's base stats.
 */
UCLASS()
class RELIQUARY_API URLCraftingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    /**
     *  Check whether a recipe can currently be crafted from the given inventory.
     *  `SelectedMaterials` optionally pins which materials satisfy family-based
     *  ingredients; leave empty to auto-select from what's on hand.
     */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool CanCraft(FName RecipeId, URLInventoryComponent* Inventory, int32 CharacterLevel,
                  const TArray<FName>& SelectedMaterials, FText& OutReason) const;

    /**
     *  Craft the recipe: consume materials from `Inventory`, roll the resulting
     *  gear, and (on success) add it to the inventory. Returns the full result.
     */
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FRLCraftResult Craft(FName RecipeId, URLInventoryComponent* Inventory, int32 CharacterLevel,
                         const TArray<FName>& SelectedMaterials);

private:
    URLItemRegistry* GetRegistry() const;

    /**
     *  Resolve the concrete material stacks a recipe will consume from an
     *  inventory. Honors explicit selections and falls back to on-hand matches
     *  for family ingredients. Returns false if requirements can't be met.
     */
    bool ResolveIngredients(const URLRecipe& Recipe, const URLInventoryComponent& Inventory,
                            const TArray<FName>& SelectedMaterials,
                            TArray<FRLMaterialStack>& OutConsumed, FText& OutReason) const;

    /** Build a rolled gear instance from a template + the materials consumed. */
    FRLGearInstance RollGear(const URLGearDefinition& Def, int32 CharacterLevel,
                             const TArray<FRLMaterialStack>& Consumed) const;
};
