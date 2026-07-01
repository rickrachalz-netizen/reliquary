// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RLTypes.h"
#include "RLMaterialDefinition.generated.h"

class UTexture2D;

/**
 *  A distinct gatherable material. There is no generic "wood": oak, ironwood,
 *  and feywood are each their own definition with their own qualities. Recipes
 *  reference these by `MaterialId` so a finished item inherits the character of
 *  whatever went into it.
 */
UCLASS(BlueprintType)
class RELIQUARY_API URLMaterialDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    /** Stable id used in saves, inventories, and recipes (e.g. "Mat.Ironwood"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FName MaterialId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLMaterialFamily Family = ERLMaterialFamily::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLRarity Rarity = ERLRarity::Common;

    /** Icon shown in the inventory. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    TObjectPtr<UTexture2D> Icon;

    /**
     *  Zones where this material reliably spawns, enabling planned routes.
     *  A player who needs ironwood knows which altar path to take.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sourcing")
    TArray<ERLZone> SourceZones;

    /**
     *  Quality contribution this material lends to a crafted item. Higher-tier
     *  materials of the same family raise an item's base stats and affix budget.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting", meta = (ClampMin = 0))
    float QualityWeight = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 1))
    int32 MaxStack = 999;
};
