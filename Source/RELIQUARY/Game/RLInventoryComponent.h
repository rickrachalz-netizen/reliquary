// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RLTypes.h"
#include "RLGearDefinition.h"
#include "RLInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRLOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRLOnEquipmentChanged, ERLGearSlot, Slot);

/**
 *  The player's persistent home stash: materials banked back to base camp,
 *  crafted gear, what's currently equipped, and which recipes are known.
 *
 *  Materials being carried mid-run live on the run subsystem's "run bag" and
 *  are only deposited here on a successful extraction or banking crate — so
 *  dying forfeits them without ever touching this stash.
 */
UCLASS(ClassGroup = (RELIQUARY), meta = (BlueprintSpawnableComponent))
class RELIQUARY_API URLInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    URLInventoryComponent();

    // ---- Events ---------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FRLOnInventoryChanged OnInventoryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FRLOnEquipmentChanged OnEquipmentChanged;

    // ---- Materials ------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Inventory|Materials")
    void AddMaterial(FName MaterialId, int32 Count);

    /** Deposit a whole batch at once (used by extraction / banking). */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Materials")
    void AddMaterials(const TArray<FRLMaterialStack>& Stacks);

    /** Returns true and removes if enough are present; otherwise returns false and removes nothing. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Materials")
    bool RemoveMaterial(FName MaterialId, int32 Count);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Materials")
    int32 GetMaterialCount(FName MaterialId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Materials")
    TArray<FRLMaterialStack> GetMaterials() const { return Materials; }

    // ---- Gear -----------------------------------------------------------

    /** Add a crafted gear instance (assigns an InstanceId if missing). Returns its id. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Gear")
    FGuid AddGear(const FRLGearInstance& Gear);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Gear")
    bool RemoveGear(FGuid InstanceId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Gear")
    bool FindGear(FGuid InstanceId, FRLGearInstance& OutGear) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Gear")
    TArray<FRLGearInstance> GetAllGear() const { return Gear; }

    // ---- Equipment ------------------------------------------------------

    /** Equip a gear instance into its slot. Returns false if not found. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool EquipGear(FGuid InstanceId);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    void UnequipSlot(ERLGearSlot Slot);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
    FGuid GetEquipped(ERLGearSlot Slot) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
    TMap<ERLGearSlot, FGuid> GetEquipment() const { return Equipped; }

    // ---- Recipes --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Inventory|Recipes")
    void LearnRecipe(FName RecipeId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Recipes")
    bool KnowsRecipe(FName RecipeId) const { return KnownRecipes.Contains(RecipeId); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Recipes")
    TArray<FName> GetKnownRecipes() const { return KnownRecipes.Array(); }

    // ---- Save/load ------------------------------------------------------

    void ResetAll();
    const TSet<FName>& GetKnownRecipeSet() const { return KnownRecipes; }
    void RestoreState(const TArray<FRLMaterialStack>& InMaterials,
                      const TArray<FRLGearInstance>& InGear,
                      const TMap<ERLGearSlot, FGuid>& InEquipped,
                      const TSet<FName>& InRecipes);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FRLMaterialStack> Materials;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FRLGearInstance> Gear;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TMap<ERLGearSlot, FGuid> Equipped;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TSet<FName> KnownRecipes;
};
