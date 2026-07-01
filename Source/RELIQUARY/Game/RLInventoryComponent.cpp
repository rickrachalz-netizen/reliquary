// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLInventoryComponent.h"
#include "RLItemRegistry.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

URLInventoryComponent::URLInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URLInventoryComponent::AddMaterial(FName MaterialId, int32 Count)
{
    if (MaterialId == NAME_None || Count <= 0)
    {
        return;
    }
    if (FRLMaterialStack* Existing = Materials.FindByPredicate(
            [MaterialId](const FRLMaterialStack& S) { return S.MaterialId == MaterialId; }))
    {
        Existing->Count += Count;
    }
    else
    {
        Materials.Emplace(MaterialId, Count);
    }
    OnInventoryChanged.Broadcast();
}

void URLInventoryComponent::AddMaterials(const TArray<FRLMaterialStack>& Stacks)
{
    bool bChanged = false;
    for (const FRLMaterialStack& Stack : Stacks)
    {
        if (Stack.MaterialId == NAME_None || Stack.Count <= 0)
        {
            continue;
        }
        if (FRLMaterialStack* Existing = Materials.FindByPredicate(
                [&Stack](const FRLMaterialStack& S) { return S.MaterialId == Stack.MaterialId; }))
        {
            Existing->Count += Stack.Count;
        }
        else
        {
            Materials.Add(Stack);
        }
        bChanged = true;
    }
    if (bChanged)
    {
        OnInventoryChanged.Broadcast();
    }
}

bool URLInventoryComponent::RemoveMaterial(FName MaterialId, int32 Count)
{
    if (Count <= 0)
    {
        return true;
    }
    const int32 Index = Materials.IndexOfByPredicate(
        [MaterialId](const FRLMaterialStack& S) { return S.MaterialId == MaterialId; });
    if (Index == INDEX_NONE || Materials[Index].Count < Count)
    {
        return false;
    }
    Materials[Index].Count -= Count;
    if (Materials[Index].Count <= 0)
    {
        Materials.RemoveAt(Index);
    }
    OnInventoryChanged.Broadcast();
    return true;
}

int32 URLInventoryComponent::GetMaterialCount(FName MaterialId) const
{
    const FRLMaterialStack* Found = Materials.FindByPredicate(
        [MaterialId](const FRLMaterialStack& S) { return S.MaterialId == MaterialId; });
    return Found ? Found->Count : 0;
}

FGuid URLInventoryComponent::AddGear(const FRLGearInstance& InGear)
{
    FRLGearInstance Copy = InGear;
    if (!Copy.InstanceId.IsValid())
    {
        Copy.InstanceId = FGuid::NewGuid();
    }
    Gear.Add(Copy);
    OnInventoryChanged.Broadcast();
    return Copy.InstanceId;
}

bool URLInventoryComponent::RemoveGear(FGuid InstanceId)
{
    // Unequip first so no slot points at a deleted item.
    for (const TPair<ERLGearSlot, FGuid>& Pair : Equipped)
    {
        if (Pair.Value == InstanceId)
        {
            UnequipSlot(Pair.Key);
            break;
        }
    }
    const int32 Removed = Gear.RemoveAll([InstanceId](const FRLGearInstance& G) { return G.InstanceId == InstanceId; });
    if (Removed > 0)
    {
        OnInventoryChanged.Broadcast();
        return true;
    }
    return false;
}

bool URLInventoryComponent::FindGear(FGuid InstanceId, FRLGearInstance& OutGear) const
{
    if (const FRLGearInstance* Found = Gear.FindByPredicate([InstanceId](const FRLGearInstance& G) { return G.InstanceId == InstanceId; }))
    {
        OutGear = *Found;
        return true;
    }
    return false;
}

bool URLInventoryComponent::EquipGear(FGuid InstanceId)
{
    FRLGearInstance Found;
    if (!FindGear(InstanceId, Found))
    {
        return false;
    }

    // The slot lives on the gear's design-time definition; resolve it through
    // the registry so the inventory stays decoupled from the content library.
    ERLGearSlot Slot = ERLGearSlot::None;
    if (const UWorld* World = GetWorld())
    {
        if (const UGameInstance* GI = World->GetGameInstance())
        {
            if (const URLItemRegistry* Registry = GI->GetSubsystem<URLItemRegistry>())
            {
                if (const URLGearDefinition* Def = Registry->GetGear(Found.GearId))
                {
                    Slot = Def->Slot;
                }
            }
        }
    }

    if (Slot == ERLGearSlot::None)
    {
        return false;
    }

    Equipped.Add(Slot, InstanceId);
    OnEquipmentChanged.Broadcast(Slot);
    return true;
}

void URLInventoryComponent::UnequipSlot(ERLGearSlot Slot)
{
    if (Equipped.Remove(Slot) > 0)
    {
        OnEquipmentChanged.Broadcast(Slot);
    }
}

FGuid URLInventoryComponent::GetEquipped(ERLGearSlot Slot) const
{
    const FGuid* Found = Equipped.Find(Slot);
    return Found ? *Found : FGuid();
}

void URLInventoryComponent::LearnRecipe(FName RecipeId)
{
    if (RecipeId != NAME_None && !KnownRecipes.Contains(RecipeId))
    {
        KnownRecipes.Add(RecipeId);
        OnInventoryChanged.Broadcast();
    }
}

void URLInventoryComponent::ResetAll()
{
    Materials.Reset();
    Gear.Reset();
    Equipped.Reset();
    KnownRecipes.Reset();
    OnInventoryChanged.Broadcast();
}

void URLInventoryComponent::RestoreState(const TArray<FRLMaterialStack>& InMaterials,
                                         const TArray<FRLGearInstance>& InGear,
                                         const TMap<ERLGearSlot, FGuid>& InEquipped,
                                         const TSet<FName>& InRecipes)
{
    Materials = InMaterials;
    Gear = InGear;
    Equipped = InEquipped;
    KnownRecipes = InRecipes;
    OnInventoryChanged.Broadcast();
}
