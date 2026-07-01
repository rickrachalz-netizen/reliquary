// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RLTypes.h"
#include "RLRunSubsystem.generated.h"

class ARLPlayerState;
class URLInventoryComponent;
class URLItemRegistry;
class URLBoonDefinition;

/** A run-power boon the player currently holds, with its stack count. */
USTRUCT(BlueprintType)
struct FRLActiveBoon
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Boon")
    FName BoonId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Boon")
    int32 Stacks = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRLOnRunPhaseChanged, ERLRunPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRLOnRunLevelChanged, int32, NewLevelIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRLOnRunBagChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRLOnBoonsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRLOnBankCrateOffered, int32, LevelIndex);

/**
 *  Owns the moment-to-moment run: embark, the ten interconnected levels, the
 *  temporary boons and excess mana that make gathering exciting, banking, and
 *  the two ways a run ends — extract (keep everything, gain XP) or die (forfeit
 *  the whole run bag). All roguelike power is stripped the instant the player
 *  is back in base camp.
 *
 *  Run RNG is rolled and locked at embark so players can't restart to fish for
 *  a better map.
 */
UCLASS()
class RELIQUARY_API URLRunSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ---- Events ---------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Run") FRLOnRunPhaseChanged  OnRunPhaseChanged;
    UPROPERTY(BlueprintAssignable, Category = "Run") FRLOnRunLevelChanged  OnRunLevelChanged;
    UPROPERTY(BlueprintAssignable, Category = "Run") FRLOnRunBagChanged    OnRunBagChanged;
    UPROPERTY(BlueprintAssignable, Category = "Run") FRLOnBoonsChanged     OnBoonsChanged;
    UPROPERTY(BlueprintAssignable, Category = "Run") FRLOnBankCrateOffered OnBankCrateOffered;

    // ---- State queries --------------------------------------------------

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    ERLRunPhase GetPhase() const { return Phase; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    int32 GetLevelIndex() const { return LevelIndex; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    ERLZone GetCurrentZone() const { return CurrentZone; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    int32 GetRunSeed() const { return RunSeed; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    float GetRunElapsedSeconds() const;

    /** The escalating difficulty scalar, driven by time and depth (RoR2-style). */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    float GetDifficultyScalar() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    TArray<FRLMaterialStack> GetRunBag() const { return RunBag; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    TArray<FRLActiveBoon> GetActiveBoons() const { return ActiveBoons; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    int32 GetBoonStacks(FName BoonId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    bool IsRunActive() const { return Phase != ERLRunPhase::InBaseCamp && Phase != ERLRunPhase::Defeated; }

    // ---- Run lifecycle --------------------------------------------------

    /** Begin a run. Rolls and locks the seed (0 = random). Called at embark. */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void Embark(int32 InSeed, ERLZone StartingZone);

    /** Harvested material goes into the run bag (kept only if extracted/banked). */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void AddRunMaterial(FName MaterialId, int32 Count);

    /** Award excess mana to the player's attribute (the run currency). */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void GrantExcessMana(float Amount);

    /**
     *  A challenge altar finished charging and its boss is dead. The player then
     *  chooses: extract to base camp, or teleport onward to a fresh map.
     */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void OnChallengeAltarCleared();

    /** Extract to base camp: bank the run bag, gain XP, strip all run power. */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void ExtractToBaseCamp();

    /** Teleport onward to the next level with a fresh map (keeps run power). */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void TeleportOnward(ERLZone NextZone);

    /** Send the current run bag home while continuing the run (offered every 3 maps). */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void UseBankCrate();

    /** The player died on the run: forfeit the run bag and strip run power. */
    UFUNCTION(BlueprintCallable, Category = "Run")
    void OnPlayerDied();

    // ---- Boons (run power) ---------------------------------------------

    /**
     *  Attempt to buy a boon at a power altar, paying excess mana. Returns false
     *  (and changes nothing) if unaffordable or maxed out.
     */
    UFUNCTION(BlueprintCallable, Category = "Run")
    bool PurchaseBoon(FName BoonId);

    /** Cost of the next stack of a boon given current holdings. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    int32 GetBoonCost(FName BoonId) const;

    /**
     *  Offer a randomized set of boons from a power altar. Deterministic given
     *  the run seed + altar id so re-opening the same altar shows the same set.
     */
    UFUNCTION(BlueprintCallable, Category = "Run")
    TArray<URLBoonDefinition*> RollBoonOffer(int32 AltarId, int32 Count) const;

private:
    void SetPhase(ERLRunPhase NewPhase);
    void DepositRunBagToHome();
    void ClearRunPower();
    void RecomputePlayerStats() const;

    ARLPlayerState* GetPlayerState() const;
    URLInventoryComponent* GetHomeInventory() const;
    URLItemRegistry* GetRegistry() const;

    ERLRunPhase Phase = ERLRunPhase::InBaseCamp;
    int32 LevelIndex = 0;
    int32 RunSeed = 0;
    int32 MapsSinceBank = 0;
    ERLZone CurrentZone = ERLZone::BaseCamp;
    double RunStartTime = 0.0;

    UPROPERTY() TArray<FRLMaterialStack> RunBag;
    UPROPERTY() TArray<FRLActiveBoon> ActiveBoons;
};
