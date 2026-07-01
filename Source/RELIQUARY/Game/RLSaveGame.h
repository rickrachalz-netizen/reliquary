// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RLTypes.h"
#include "RLGearDefinition.h"
#include "RLSaveGame.generated.h"

/**
 *  The persistent profile written to disk. Captures everything about a hero
 *  that survives between sessions: identity, progression, home stash, and gear.
 *
 *  Nothing about an in-progress run is stored — active runs can't be saved or
 *  reloaded, by design. The game autosaves whenever the player is in base camp.
 */
UCLASS()
class RELIQUARY_API URLSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, Category = "Meta")
    int32 SaveVersion = 1;

    // ---- Identity & progression ----------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    FString HeroName = TEXT("Nameless Scavenger");

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    ERLClass CharacterClass = ERLClass::Warrior;

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    ERLSpec ActiveSpec = ERLSpec::None;

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    int32 Level = 1;

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    int32 Experience = 0;

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    int32 UnspentTalentPoints = 0;

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    int32 EssenceLevel = 0;

    UPROPERTY(VisibleAnywhere, Category = "Hero")
    int32 EssenceProgress = 0;

    // ---- Home stash -----------------------------------------------------

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FRLMaterialStack> Materials;

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FRLGearInstance> Gear;

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TMap<ERLGearSlot, FGuid> Equipped;

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FName> KnownRecipes;
};
