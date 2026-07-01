// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RLTypes.generated.h"

/**
 *  Shared enums and light-weight structs for RELIQUARY's gameplay systems.
 *  Kept free of behaviour so every subsystem can depend on it cheaply.
 */

/** The three base classes chosen at character creation. */
UENUM(BlueprintType)
enum class ERLClass : uint8
{
    None    UMETA(DisplayName = "None"),
    Warrior UMETA(DisplayName = "Warrior"),
    Rogue   UMETA(DisplayName = "Rogue"),
    Mage    UMETA(DisplayName = "Mage")
};

/**
 *  Talent-tree specialisations. Each base class owns three specs; the active
 *  spec (plus talent choices) drives the "evolving identity" class name shown
 *  in the UI at max level.
 */
UENUM(BlueprintType)
enum class ERLSpec : uint8
{
    None            UMETA(DisplayName = "None"),
    // Warrior
    Juggernaut      UMETA(DisplayName = "Juggernaut"),   // sustain / control
    Berserker       UMETA(DisplayName = "Berserker"),    // reckless burst
    Warden          UMETA(DisplayName = "Warden"),       // protective / thorns
    // Rogue
    Duelist         UMETA(DisplayName = "Duelist"),      // precision single-target
    Shadowblade     UMETA(DisplayName = "Shadowblade"),  // stealth / poison
    Trickster       UMETA(DisplayName = "Trickster"),    // mobility / gadgets
    // Mage
    Pyromancer      UMETA(DisplayName = "Pyromancer"),   // fire / ignite
    Frostweaver     UMETA(DisplayName = "Frostweaver"),  // control / shatter
    Stormcaller     UMETA(DisplayName = "Stormcaller")   // chain / tempo
};

/** Which primary stat a class scales from. Drives derived offense. */
UENUM(BlueprintType)
enum class ERLPrimaryStat : uint8
{
    Strength  UMETA(DisplayName = "Strength"),
    Agility   UMETA(DisplayName = "Agility"),
    Intellect UMETA(DisplayName = "Intellect")
};

/** Item quality. Higher rarities carry more affixes and set potential. */
UENUM(BlueprintType)
enum class ERLRarity : uint8
{
    Common    UMETA(DisplayName = "Common"),
    Uncommon  UMETA(DisplayName = "Uncommon"),
    Rare      UMETA(DisplayName = "Rare"),
    Epic      UMETA(DisplayName = "Epic"),
    Legendary UMETA(DisplayName = "Legendary"),
    Relic     UMETA(DisplayName = "Relic")        // reserved for end-boss drops
};

/** Equipment slots. */
UENUM(BlueprintType)
enum class ERLGearSlot : uint8
{
    None      UMETA(DisplayName = "None"),
    Weapon    UMETA(DisplayName = "Weapon"),
    Offhand   UMETA(DisplayName = "Offhand"),
    Head      UMETA(DisplayName = "Head"),
    Chest     UMETA(DisplayName = "Chest"),
    Hands     UMETA(DisplayName = "Hands"),
    Legs      UMETA(DisplayName = "Legs"),
    Feet      UMETA(DisplayName = "Feet"),
    Trinket   UMETA(DisplayName = "Trinket")
};

/** Broad weapon archetypes. */
UENUM(BlueprintType)
enum class ERLWeaponType : uint8
{
    None      UMETA(DisplayName = "None"),
    Sword     UMETA(DisplayName = "Sword"),
    Greatsword UMETA(DisplayName = "Greatsword"),
    Dagger    UMETA(DisplayName = "Dagger"),
    Bow       UMETA(DisplayName = "Bow"),
    Staff     UMETA(DisplayName = "Staff"),
    Wand      UMETA(DisplayName = "Wand")
};

/**
 *  The material families that gatherable resources belong to. Every concrete
 *  material (oak, ironwood, feywood...) is a distinct item, but grouping them
 *  by family lets recipes and resource nodes reason generically.
 */
UENUM(BlueprintType)
enum class ERLMaterialFamily : uint8
{
    None    UMETA(DisplayName = "None"),
    Wood    UMETA(DisplayName = "Wood"),
    Stone   UMETA(DisplayName = "Stone"),
    Ore     UMETA(DisplayName = "Ore"),
    Hide    UMETA(DisplayName = "Hide"),
    Essence UMETA(DisplayName = "Essence"),   // condensed realm energy
    Bone    UMETA(DisplayName = "Bone"),
    Relic   UMETA(DisplayName = "Relic")      // boss-only crafting material
};

/**
 *  The realm's ten interconnected zones. Materials reliably spawn in certain
 *  zones so players can plan routes ("ironwood grows in the Sunken Wold").
 */
UENUM(BlueprintType)
enum class ERLZone : uint8
{
    None            UMETA(DisplayName = "None"),
    BaseCamp        UMETA(DisplayName = "Base Camp"),          // the fallen god's corpse
    WhisperingGrove UMETA(DisplayName = "Whispering Grove"),
    SunkenWold      UMETA(DisplayName = "Sunken Wold"),
    AshenReach      UMETA(DisplayName = "Ashen Reach"),
    GlassSteppes    UMETA(DisplayName = "Glass Steppes"),
    HollowFen       UMETA(DisplayName = "Hollow Fen"),
    RimewoodVale    UMETA(DisplayName = "Rimewood Vale"),
    TheScar         UMETA(DisplayName = "The Scar"),
    GildedRuin      UMETA(DisplayName = "Gilded Ruin"),
    GodsThreshold   UMETA(DisplayName = "God's Threshold")     // final-boss approach
};

/** How the current run is progressing through the challenge-altar path. */
UENUM(BlueprintType)
enum class ERLRunPhase : uint8
{
    InBaseCamp  UMETA(DisplayName = "In Base Camp"),  // safe; run power stripped
    Exploring   UMETA(DisplayName = "Exploring"),     // gathering, spending mana
    AltarActive UMETA(DisplayName = "Altar Active"),  // charging altar, boss engaged
    Transition  UMETA(DisplayName = "Transition"),    // extracting or teleporting on
    Defeated    UMETA(DisplayName = "Defeated")       // died; resources forfeit
};

/** What a run-power boon does, at a high level. Details live on the boon asset. */
UENUM(BlueprintType)
enum class ERLBoonKind : uint8
{
    None            UMETA(DisplayName = "None"),
    FlatStat        UMETA(DisplayName = "Flat Stat"),        // + primary/secondary
    PercentStat     UMETA(DisplayName = "Percent Stat"),     // x multiplier
    OnHitEffect     UMETA(DisplayName = "On Hit Effect"),    // proc
    Utility         UMETA(DisplayName = "Utility"),          // movement, mana, etc.
    Cursed          UMETA(DisplayName = "Cursed")            // power with a downside
};

/**
 *  A material stack the player is carrying. `MaterialId` maps to a
 *  URLMaterialDefinition (a data asset), keeping saves small and stable.
 */
USTRUCT(BlueprintType)
struct FRLMaterialStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    FName MaterialId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    int32 Count = 0;

    FRLMaterialStack() = default;
    FRLMaterialStack(FName InId, int32 InCount) : MaterialId(InId), Count(InCount) {}
};
