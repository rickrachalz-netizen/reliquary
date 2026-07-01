// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RLTypes.h"
#include "RLGearDefinition.h" // FRLStatModifier
#include "RLBoonDefinition.generated.h"

class UTexture2D;

/**
 *  A run-power upgrade offered at a power altar. Boons are the roguelike layer:
 *  bought with excess mana during a run, stacked to absurd heights, and wiped
 *  the instant the player returns to base camp. They exist to make gathering
 *  exciting, not to replace permanent progression.
 */
UCLASS(BlueprintType)
class RELIQUARY_API URLBoonDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FName BoonId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLBoonKind Kind = ERLBoonKind::FlatStat;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLRarity Rarity = ERLRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    TObjectPtr<UTexture2D> Icon;

    /** Excess-mana cost the first time this boon is offered. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = 0))
    int32 BaseCost = 50;

    /** How much each already-owned stack raises the cost of the next one. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = 0))
    float CostGrowthPerStack = 0.5f;

    /** Times this boon may be stacked in a single run (0 = unlimited). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = 0))
    int32 MaxStacks = 0;

    /** Stat changes applied per stack while the boon is held. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    TArray<FRLStatModifier> Modifiers;

    /** Tag granted while held, for proc/utility boons abilities react to. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
    FGameplayTag GrantedTag;

    /** For Cursed boons: a short line describing the downside, surfaced in UI. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (EditCondition = "Kind == ERLBoonKind::Cursed"))
    FText DownsideText;

    /** Cost to buy the next stack given how many are already owned. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cost")
    int32 GetCostForStack(int32 OwnedStacks) const
    {
        const float Mult = 1.f + CostGrowthPerStack * FMath::Max(0, OwnedStacks);
        return FMath::RoundToInt(BaseCost * Mult);
    }
};
