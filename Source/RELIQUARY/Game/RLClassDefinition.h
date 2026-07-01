// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RLTypes.h"
#include "RLClassDefinition.generated.h"

/** Base attribute block for a class at level 1, plus its per-level growth. */
USTRUCT(BlueprintType)
struct FRLBaseStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
    float MaxMana = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
    float HealthRegen = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
    float ManaRegen = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primary")
    float Strength = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primary")
    float Agility = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primary")
    float Intellect = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
    float CritChance = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
    float Haste = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Secondary")
    float Adaptability = 0.02f;
};

/**
 *  Design-time definition of one of the three base classes.
 *  Authored as a data asset so class tuning lives in content, not code.
 */
UCLASS(BlueprintType)
class RELIQUARY_API URLClassDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLClass ClassId = ERLClass::Warrior;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
    FText Description;

    /** Which primary stat this class scales its offense from. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLPrimaryStat ScalingStat = ERLPrimaryStat::Strength;

    /** The three specs (talent trees) available to this class. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
    TArray<ERLSpec> Specs;

    /** Stats at level 1. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FRLBaseStats BaseStats;

    /** Additive stat gain applied per level above 1. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FRLBaseStats PerLevelGrowth;

    /**
     *  Compute the fully grown stat block for a given level (1..MaxLevel).
     *  Growth is linear per level on top of the level-1 base.
     */
    UFUNCTION(BlueprintCallable, Category = "Stats")
    FRLBaseStats GetStatsForLevel(int32 Level) const;
};
