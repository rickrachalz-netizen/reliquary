// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RLTypes.h"
#include "RLPlayerState.generated.h"

class URLClassDefinition;
class URLAttributeSet;
class UAbilitySystemComponent;
class URLInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRLOnLevelChanged, int32, NewLevel, int32, TalentPointsGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRLOnExperienceChanged, int32, CurrentXP, int32, XPToNextLevel, int32, Level);

/**
 *  Persistent, player-created hero data.
 *
 *  Owns the character's identity (class + spec), level 1..30 progression with
 *  an exponential XP curve, talent points, and the WoW:BfA-style essence
 *  ("Heart") level. Applies the resulting base stats onto the pawn's GAS
 *  attribute set whenever identity or level changes.
 *
 *  Note: run-gathered materials/gear live in the inventory component and the
 *  save game; this class is purely the character sheet.
 */
UCLASS()
class RELIQUARY_API ARLPlayerState : public APlayerState
{
    GENERATED_BODY()
public:
    ARLPlayerState();

    /** Hard cap on character level, per the design doc. */
    static constexpr int32 MaxLevel = 30;

    // ---- Identity -------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Identity, Category = "Identity")
    ERLClass CharacterClass = ERLClass::Warrior;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Identity, Category = "Identity")
    ERLSpec ActiveSpec = ERLSpec::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString HeroName = TEXT("Nameless Scavenger");

    /**
     *  Class tuning tables, one per base class. Optional override; when empty the
     *  definitions are resolved from the item registry (the content library).
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    TMap<ERLClass, TObjectPtr<URLClassDefinition>> ClassDefinitions;

    /** The persistent home stash / equipment, owned by the hero. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<URLInventoryComponent> Inventory;

    // ---- Progression ----------------------------------------------------

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "Progression")
    int32 Level = 1;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Progression")
    int32 Experience = 0;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Progression")
    int32 UnspentTalentPoints = 0;

    /** Heart-of-the-God essence level: a parallel, slow long-term track. */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Progression")
    int32 EssenceLevel = 0;

    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Progression")
    int32 EssenceProgress = 0;

    // ---- Events ---------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Progression")
    FRLOnLevelChanged OnLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Progression")
    FRLOnExperienceChanged OnExperienceChanged;

    // ---- Queries --------------------------------------------------------

    /** XP required to advance from `Level` to `Level+1`. Exponential, fast early. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progression")
    static int32 GetExperienceForLevel(int32 InLevel);

    /** XP remaining to the next level (0 at max level). */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progression")
    int32 GetExperienceToNextLevel() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progression")
    bool IsMaxLevel() const { return Level >= MaxLevel; }

    /**
     *  The class name shown in the UI. Before max level it's the base class;
     *  at max level it becomes the spec-derived "evolved" identity.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Identity")
    FText GetDisplayClassName() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Identity")
    URLClassDefinition* GetActiveClassDefinition() const;

    // ---- Mutation -------------------------------------------------------

    /** Grant XP (typically on a successful extraction) and process level-ups. */
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void GrantExperience(int32 Amount);

    /** Grant essence toward the long-term Heart track. */
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void GrantEssence(int32 Amount);

    /** Pick class + starting spec at character creation, then apply base stats. */
    UFUNCTION(BlueprintCallable, Category = "Identity")
    void InitializeCharacter(ERLClass InClass, ERLSpec InSpec);

    /**
     *  Recompute and apply the character's full stat block onto the owning pawn:
     *  class/level base + equipped gear + active run boons. When bResetVitals is
     *  true (level-up, load, entering base camp) Health/Mana are refilled;
     *  otherwise current values are preserved so a mid-run boon doesn't heal you.
     */
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void ApplyStatsToOwner(bool bResetVitals = true);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    URLInventoryComponent* GetInventory() const { return Inventory; }

    // ---- Excess mana (run currency) ------------------------------------

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
    float GetExcessMana() const;

    UFUNCTION(BlueprintCallable, Category = "Run")
    void AddExcessMana(float Amount);

    /** Spend excess mana if affordable; returns false and spends nothing otherwise. */
    UFUNCTION(BlueprintCallable, Category = "Run")
    bool SpendExcessMana(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Run")
    void SetExcessMana(float Amount);

    // ---- Save/load helpers ---------------------------------------------

    void LoadFrom(ERLClass InClass, ERLSpec InSpec, int32 InLevel, int32 InXP,
                  int32 InTalentPoints, int32 InEssenceLevel, int32 InEssenceProgress,
                  const FString& InHeroName);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION() void OnRep_Level();
    UFUNCTION() void OnRep_Identity();

    /** Resolve the ASC/attribute set on the currently controlled pawn. */
    URLAttributeSet* ResolveAttributeSet(UAbilitySystemComponent*& OutASC) const;

    /** How many talent points a level grants (max level also grants one final point). */
    static int32 TalentPointsForLevel(int32 NewLevel);
};
