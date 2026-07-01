// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RLTypes.h"
#include "RLGearDefinition.generated.h"

class UTexture2D;
class UStaticMesh;

/** A flat or percent modifier to a named attribute, used by affixes and gear. */
USTRUCT(BlueprintType)
struct FRLStatModifier
{
    GENERATED_BODY()

    /** Attribute name, matching a URLAttributeSet property (e.g. "Strength"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    FName AttributeName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    float FlatBonus = 0.f;

    /** Multiplicative bonus (0.10 == +10%). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
    float PercentBonus = 0.f;
};

/**
 *  Design-time template for a piece of gear. A crafted item pairs this template
 *  with rolled affixes and the materials used (see FRLGearInstance). Base stats
 *  start modest but scale hard through affixes, cantrips, and set bonuses.
 */
UCLASS(BlueprintType)
class RELIQUARY_API URLGearDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FName GearId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLGearSlot Slot = ERLGearSlot::Weapon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (EditCondition = "Slot == ERLGearSlot::Weapon"))
    ERLWeaponType WeaponType = ERLWeaponType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    TObjectPtr<UTexture2D> Icon;

    /** Optional mesh to attach when equipped. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
    TSoftObjectPtr<UStaticMesh> WorldMesh;

    /** Guaranteed stats this template always grants. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    TArray<FRLStatModifier> BaseModifiers;

    /**
     *  A "cantrip": a small always-on gameplay effect described by a tag that
     *  abilities/passives react to (e.g. "Cantrip.Lifesteal"). These are what
     *  make upgrades feel impactful and sought-after.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cantrip")
    FGameplayTag CantripTag;

    /** Set membership; equipping enough pieces of a set unlocks its bonuses. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Set")
    FGameplayTag SetTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLRarity BaseRarity = ERLRarity::Common;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    int32 RequiredLevel = 1;
};

/**
 *  A concrete, crafted instance of a gear template. This is what actually lives
 *  in inventories and save files: a reference to the template, its rolled
 *  rarity, rolled affixes, and the material fingerprint it was made from.
 */
USTRUCT(BlueprintType)
struct FRLGearInstance
{
    GENERATED_BODY()

    /** Unique runtime/save id so two identical crafts are still distinguishable. */
    UPROPERTY(BlueprintReadWrite, Category = "Gear")
    FGuid InstanceId;

    /** The template's GearId. */
    UPROPERTY(BlueprintReadWrite, Category = "Gear")
    FName GearId = NAME_None;

    UPROPERTY(BlueprintReadWrite, Category = "Gear")
    ERLRarity Rarity = ERLRarity::Common;

    /** Rolled affixes on top of the template's base modifiers. */
    UPROPERTY(BlueprintReadWrite, Category = "Gear")
    TArray<FRLStatModifier> RolledAffixes;

    /** Item power, derived at craft time from level + material quality. */
    UPROPERTY(BlueprintReadWrite, Category = "Gear")
    int32 ItemPower = 0;

    /** Materials that went into this craft (for provenance / flavour / reroll). */
    UPROPERTY(BlueprintReadWrite, Category = "Gear")
    TArray<FRLMaterialStack> CraftedFrom;

    bool IsValid() const { return GearId != NAME_None; }
};
