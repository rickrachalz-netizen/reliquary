// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLCraftingSubsystem.h"
#include "RLItemRegistry.h"
#include "RLInventoryComponent.h"
#include "RLRecipe.h"
#include "RLMaterialDefinition.h"

URLItemRegistry* URLCraftingSubsystem::GetRegistry() const
{
    return GetGameInstance() ? GetGameInstance()->GetSubsystem<URLItemRegistry>() : nullptr;
}

bool URLCraftingSubsystem::ResolveIngredients(const URLRecipe& Recipe, const URLInventoryComponent& Inventory,
                                              const TArray<FName>& SelectedMaterials,
                                              TArray<FRLMaterialStack>& OutConsumed, FText& OutReason) const
{
    OutConsumed.Reset();
    URLItemRegistry* Registry = GetRegistry();
    if (!Registry)
    {
        OutReason = NSLOCTEXT("RL", "NoRegistry", "Crafting data is not loaded.");
        return false;
    }

    // Track running usage so overlapping ingredients don't double-spend a stack.
    TMap<FName, int32> PlannedUse;

    for (const FRLRecipeIngredient& Ingredient : Recipe.Ingredients)
    {
        int32 Remaining = Ingredient.Count;

        // 1) A specific material was named by the recipe.
        if (Ingredient.MaterialId != NAME_None)
        {
            const int32 Available = Inventory.GetMaterialCount(Ingredient.MaterialId) - PlannedUse.FindRef(Ingredient.MaterialId);
            if (Available < Remaining)
            {
                OutReason = FText::Format(NSLOCTEXT("RL", "MissingMat", "Not enough {0}."),
                                          FText::FromName(Ingredient.MaterialId));
                return false;
            }
            PlannedUse.FindOrAdd(Ingredient.MaterialId) += Remaining;
            OutConsumed.Emplace(Ingredient.MaterialId, Remaining);
            continue;
        }

        // 2) Family ingredient — prefer the player's explicit selection.
        for (FName SelId : SelectedMaterials)
        {
            if (Remaining <= 0)
            {
                break;
            }
            const URLMaterialDefinition* Def = Registry->GetMaterial(SelId);
            if (!Def || Def->Family != Ingredient.Family)
            {
                continue;
            }
            const int32 Available = Inventory.GetMaterialCount(SelId) - PlannedUse.FindRef(SelId);
            const int32 Take = FMath::Min(Available, Remaining);
            if (Take > 0)
            {
                PlannedUse.FindOrAdd(SelId) += Take;
                OutConsumed.Emplace(SelId, Take);
                Remaining -= Take;
            }
        }

        // 3) Fall back to anything on hand of the right family.
        if (Remaining > 0)
        {
            for (const FRLMaterialStack& Stack : Inventory.GetMaterials())
            {
                if (Remaining <= 0)
                {
                    break;
                }
                const URLMaterialDefinition* Def = Registry->GetMaterial(Stack.MaterialId);
                if (!Def || Def->Family != Ingredient.Family)
                {
                    continue;
                }
                const int32 Available = Stack.Count - PlannedUse.FindRef(Stack.MaterialId);
                const int32 Take = FMath::Min(Available, Remaining);
                if (Take > 0)
                {
                    PlannedUse.FindOrAdd(Stack.MaterialId) += Take;
                    OutConsumed.Emplace(Stack.MaterialId, Take);
                    Remaining -= Take;
                }
            }
        }

        if (Remaining > 0)
        {
            OutReason = FText::Format(NSLOCTEXT("RL", "MissingFamily", "Not enough materials of the required type ({0})."),
                                      FText::AsNumber(static_cast<int32>(Ingredient.Family)));
            return false;
        }
    }

    return true;
}

bool URLCraftingSubsystem::CanCraft(FName RecipeId, URLInventoryComponent* Inventory, int32 CharacterLevel,
                                    const TArray<FName>& SelectedMaterials, FText& OutReason) const
{
    URLItemRegistry* Registry = GetRegistry();
    if (!Registry || !Inventory)
    {
        OutReason = NSLOCTEXT("RL", "CraftUnavailable", "Crafting is unavailable.");
        return false;
    }
    const URLRecipe* Recipe = Registry->GetRecipe(RecipeId);
    if (!Recipe)
    {
        OutReason = NSLOCTEXT("RL", "NoRecipe", "Unknown recipe.");
        return false;
    }
    if (CharacterLevel < Recipe->RequiredLevel)
    {
        OutReason = FText::Format(NSLOCTEXT("RL", "LevelReq", "Requires level {0}."), FText::AsNumber(Recipe->RequiredLevel));
        return false;
    }
    if (!Recipe->bKnownByDefault && !Inventory->KnowsRecipe(RecipeId))
    {
        OutReason = NSLOCTEXT("RL", "RecipeUnknown", "You haven't learned this recipe.");
        return false;
    }

    TArray<FRLMaterialStack> Consumed;
    return ResolveIngredients(*Recipe, *Inventory, SelectedMaterials, Consumed, OutReason);
}

FRLCraftResult URLCraftingSubsystem::Craft(FName RecipeId, URLInventoryComponent* Inventory, int32 CharacterLevel,
                                           const TArray<FName>& SelectedMaterials)
{
    FRLCraftResult Result;

    URLItemRegistry* Registry = GetRegistry();
    if (!Registry || !Inventory)
    {
        Result.FailureReason = NSLOCTEXT("RL", "CraftUnavailable", "Crafting is unavailable.");
        return Result;
    }
    const URLRecipe* Recipe = Registry->GetRecipe(RecipeId);
    if (!Recipe)
    {
        Result.FailureReason = NSLOCTEXT("RL", "NoRecipe", "Unknown recipe.");
        return Result;
    }

    TArray<FRLMaterialStack> Consumed;
    if (!CanCraft(RecipeId, Inventory, CharacterLevel, SelectedMaterials, Result.FailureReason) ||
        !ResolveIngredients(*Recipe, *Inventory, SelectedMaterials, Consumed, Result.FailureReason))
    {
        return Result;
    }

    const URLGearDefinition* Def = Registry->GetGear(Recipe->OutputGearId);
    if (!Def)
    {
        Result.FailureReason = NSLOCTEXT("RL", "NoOutput", "This recipe's output is missing.");
        return Result;
    }

    // Commit: consume the materials, then roll and grant the item.
    for (const FRLMaterialStack& Stack : Consumed)
    {
        Inventory->RemoveMaterial(Stack.MaterialId, Stack.Count);
    }

    Result.CraftedItem = RollGear(*Def, CharacterLevel, Consumed);
    Result.CraftedItem.InstanceId = Inventory->AddGear(Result.CraftedItem);
    Result.bSuccess = true;
    return Result;
}

FRLGearInstance URLCraftingSubsystem::RollGear(const URLGearDefinition& Def, int32 CharacterLevel,
                                               const TArray<FRLMaterialStack>& Consumed) const
{
    URLItemRegistry* Registry = GetRegistry();

    // Weighted-average material quality drives item power and rarity.
    float QualitySum = 0.f;
    int32 QualityCount = 0;
    ERLRarity BestMaterialRarity = ERLRarity::Common;
    for (const FRLMaterialStack& Stack : Consumed)
    {
        const URLMaterialDefinition* MatDef = Registry ? Registry->GetMaterial(Stack.MaterialId) : nullptr;
        const float Weight = MatDef ? MatDef->QualityWeight : 1.f;
        QualitySum += Weight * Stack.Count;
        QualityCount += Stack.Count;
        if (MatDef && static_cast<uint8>(MatDef->Rarity) > static_cast<uint8>(BestMaterialRarity))
        {
            BestMaterialRarity = MatDef->Rarity;
        }
    }
    const float AvgQuality = (QualityCount > 0) ? QualitySum / QualityCount : 1.f;

    FRLGearInstance Instance;
    Instance.InstanceId = FGuid::NewGuid();
    Instance.GearId = Def.GearId;
    Instance.CraftedFrom = Consumed;

    // Item power: scales with character level and material quality.
    Instance.ItemPower = FMath::RoundToInt((10 + CharacterLevel * 5) * FMath::Max(0.25f, AvgQuality));

    // Rarity: start from the better of the template's base and the material,
    // then give high-quality crafts a chance to bump up a tier.
    uint8 RarityIndex = FMath::Max(static_cast<uint8>(Def.BaseRarity), static_cast<uint8>(BestMaterialRarity));
    if (AvgQuality >= 2.f && FMath::FRand() < FMath::Min(0.75f, 0.25f * AvgQuality))
    {
        RarityIndex = FMath::Min<uint8>(RarityIndex + 1, static_cast<uint8>(ERLRarity::Legendary));
    }
    Instance.Rarity = static_cast<ERLRarity>(RarityIndex);

    // Affix count grows with rarity; each affix scales with item power.
    const int32 AffixCount = FMath::Clamp(static_cast<int32>(Instance.Rarity), 0, 5);
    static const FName AffixPool[] = {
        TEXT("Strength"), TEXT("Agility"), TEXT("Intellect"),
        TEXT("CritChance"), TEXT("Haste"), TEXT("MaxHealth"), TEXT("Adaptability")
    };
    for (int32 i = 0; i < AffixCount; ++i)
    {
        FRLStatModifier Affix;
        Affix.AttributeName = AffixPool[FMath::RandRange(0, UE_ARRAY_COUNT(AffixPool) - 1)];
        if (Affix.AttributeName == TEXT("CritChance"))
        {
            Affix.FlatBonus = 0.01f + 0.01f * FMath::FRand();          // +1%..+2% crit
        }
        else if (Affix.AttributeName == TEXT("Haste") || Affix.AttributeName == TEXT("Adaptability"))
        {
            Affix.PercentBonus = 0.02f + 0.03f * FMath::FRand();        // small % multipliers
        }
        else
        {
            Affix.FlatBonus = FMath::RoundToFloat(Instance.ItemPower * (0.25f + 0.5f * FMath::FRand()));
        }
        Instance.RolledAffixes.Add(Affix);
    }

    return Instance;
}
