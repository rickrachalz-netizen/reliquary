// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLPlayerState.h"
#include "RLClassDefinition.h"
#include "RLAttributeSet.h"
#include "RLInventoryComponent.h"
#include "RLItemRegistry.h"
#include "RLGearDefinition.h"
#include "RLBoonDefinition.h"
#include "RLRunSubsystem.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ARLPlayerState::ARLPlayerState()
{
    // Persistent hero data changes rarely; a modest update rate is plenty.
    SetNetUpdateFrequency(4.f);

    // The home stash / equipment travels with the hero's persistent data.
    Inventory = CreateDefaultSubobject<URLInventoryComponent>(TEXT("Inventory"));
}

void ARLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ARLPlayerState, CharacterClass);
    DOREPLIFETIME(ARLPlayerState, ActiveSpec);
    DOREPLIFETIME(ARLPlayerState, Level);
    DOREPLIFETIME(ARLPlayerState, Experience);
    DOREPLIFETIME(ARLPlayerState, UnspentTalentPoints);
    DOREPLIFETIME(ARLPlayerState, EssenceLevel);
    DOREPLIFETIME(ARLPlayerState, EssenceProgress);
}

int32 ARLPlayerState::GetExperienceForLevel(int32 InLevel)
{
    // Exponential curve: cheap early levels, steep late ones, but never a slog.
    //   cost(L) = 100 * 1.28^(L-1)
    // e.g. L1->2 = 100, L10->11 ~= 950, L29->30 ~= 76k.
    if (InLevel < 1 || InLevel >= MaxLevel)
    {
        return 0;
    }
    constexpr float Base = 100.f;
    constexpr float Growth = 1.28f;
    return FMath::RoundToInt(Base * FMath::Pow(Growth, static_cast<float>(InLevel - 1)));
}

int32 ARLPlayerState::GetExperienceToNextLevel() const
{
    const int32 Needed = GetExperienceForLevel(Level);
    return (Needed <= 0) ? 0 : FMath::Max(0, Needed - Experience);
}

int32 ARLPlayerState::TalentPointsForLevel(int32 NewLevel)
{
    // One point every even level keeps the trees from filling instantly while
    // still handing out ~15 points by max level.
    return (NewLevel % 2 == 0) ? 1 : 0;
}

void ARLPlayerState::GrantExperience(int32 Amount)
{
    if (Amount <= 0 || IsMaxLevel())
    {
        OnExperienceChanged.Broadcast(Experience, GetExperienceForLevel(Level), Level);
        return;
    }

    Experience += Amount;

    int32 GainedTalentPoints = 0;
    bool bLeveled = false;

    while (!IsMaxLevel())
    {
        const int32 Needed = GetExperienceForLevel(Level);
        if (Needed <= 0 || Experience < Needed)
        {
            break;
        }
        Experience -= Needed;
        ++Level;
        bLeveled = true;

        const int32 Points = TalentPointsForLevel(Level);
        UnspentTalentPoints += Points;
        GainedTalentPoints += Points;
    }

    if (IsMaxLevel())
    {
        // No overflow banking past the cap.
        Experience = 0;
    }

    if (bLeveled)
    {
        ApplyStatsToOwner();
        OnLevelChanged.Broadcast(Level, GainedTalentPoints);
    }

    OnExperienceChanged.Broadcast(Experience, GetExperienceForLevel(Level), Level);
}

void ARLPlayerState::GrantEssence(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }
    EssenceProgress += Amount;

    // Each essence level costs a bit more than the last; this is a slow,
    // long-term track that persists across the whole game.
    auto EssenceCost = [](int32 Lvl) { return 500 + Lvl * 250; };
    while (EssenceProgress >= EssenceCost(EssenceLevel))
    {
        EssenceProgress -= EssenceCost(EssenceLevel);
        ++EssenceLevel;
    }
}

URLClassDefinition* ARLPlayerState::GetActiveClassDefinition() const
{
    // Prefer an explicit override on the PlayerState BP...
    if (const TObjectPtr<URLClassDefinition>* Found = ClassDefinitions.Find(CharacterClass))
    {
        if (*Found)
        {
            return *Found;
        }
    }
    // ...otherwise resolve from the shared content library.
    if (const UWorld* World = GetWorld())
    {
        if (const UGameInstance* GI = World->GetGameInstance())
        {
            if (const URLItemRegistry* Registry = GI->GetSubsystem<URLItemRegistry>())
            {
                return Registry->GetClassDefinition(CharacterClass);
            }
        }
    }
    return nullptr;
}

FText ARLPlayerState::GetDisplayClassName() const
{
    // Before max level: the plain base-class name.
    if (!IsMaxLevel() || ActiveSpec == ERLSpec::None)
    {
        switch (CharacterClass)
        {
        case ERLClass::Warrior: return NSLOCTEXT("RL", "Warrior", "Warrior");
        case ERLClass::Rogue:   return NSLOCTEXT("RL", "Rogue", "Rogue");
        case ERLClass::Mage:    return NSLOCTEXT("RL", "Mage", "Mage");
        default:                return NSLOCTEXT("RL", "Adventurer", "Adventurer");
        }
    }

    // At max level the identity "evolves" to reflect the chosen spec.
    switch (ActiveSpec)
    {
    case ERLSpec::Juggernaut:  return NSLOCTEXT("RL", "EvoJuggernaut", "Unbroken Juggernaut");
    case ERLSpec::Berserker:   return NSLOCTEXT("RL", "EvoBerserker", "Blood-Mad Berserker");
    case ERLSpec::Warden:      return NSLOCTEXT("RL", "EvoWarden", "Iron Warden");
    case ERLSpec::Duelist:     return NSLOCTEXT("RL", "EvoDuelist", "Peerless Duelist");
    case ERLSpec::Shadowblade: return NSLOCTEXT("RL", "EvoShadow", "Silent Shadowblade");
    case ERLSpec::Trickster:   return NSLOCTEXT("RL", "EvoTrickster", "Fateweaving Trickster");
    case ERLSpec::Pyromancer:  return NSLOCTEXT("RL", "EvoPyro", "Ashen Pyromancer");
    case ERLSpec::Frostweaver: return NSLOCTEXT("RL", "EvoFrost", "Deep Frostweaver");
    case ERLSpec::Stormcaller:  return NSLOCTEXT("RL", "EvoStorm", "Skyborn Stormcaller");
    default:                    return GetDisplayClassName();
    }
}

void ARLPlayerState::InitializeCharacter(ERLClass InClass, ERLSpec InSpec)
{
    CharacterClass = InClass;
    ActiveSpec = InSpec;
    Level = 1;
    Experience = 0;
    UnspentTalentPoints = 0;
    ApplyStatsToOwner();
    OnLevelChanged.Broadcast(Level, 0);
    OnExperienceChanged.Broadcast(Experience, GetExperienceForLevel(Level), Level);
}

void ARLPlayerState::LoadFrom(ERLClass InClass, ERLSpec InSpec, int32 InLevel, int32 InXP,
                              int32 InTalentPoints, int32 InEssenceLevel, int32 InEssenceProgress,
                              const FString& InHeroName)
{
    CharacterClass = InClass;
    ActiveSpec = InSpec;
    Level = FMath::Clamp(InLevel, 1, MaxLevel);
    Experience = FMath::Max(0, InXP);
    UnspentTalentPoints = FMath::Max(0, InTalentPoints);
    EssenceLevel = FMath::Max(0, InEssenceLevel);
    EssenceProgress = FMath::Max(0, InEssenceProgress);
    if (!InHeroName.IsEmpty())
    {
        HeroName = InHeroName;
    }
    ApplyStatsToOwner();
    OnLevelChanged.Broadcast(Level, 0);
    OnExperienceChanged.Broadcast(Experience, GetExperienceForLevel(Level), Level);
}

URLAttributeSet* ARLPlayerState::ResolveAttributeSet(UAbilitySystemComponent*& OutASC) const
{
    OutASC = nullptr;
    APawn* OwnerPawn = GetPawn();
    if (!OwnerPawn)
    {
        return nullptr;
    }
    if (IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(OwnerPawn))
    {
        OutASC = AbilityInterface->GetAbilitySystemComponent();
    }
    if (!OutASC)
    {
        return nullptr;
    }
    return const_cast<URLAttributeSet*>(OutASC->GetSet<URLAttributeSet>());
}

void ARLPlayerState::ApplyStatsToOwner(bool bResetVitals)
{
    UAbilitySystemComponent* ASC = nullptr;
    URLAttributeSet* Attr = ResolveAttributeSet(ASC);
    if (!Attr)
    {
        return;
    }

    URLClassDefinition* Def = GetActiveClassDefinition();
    const FRLBaseStats Stats = Def ? Def->GetStatsForLevel(Level) : FRLBaseStats();

    // Accumulate flat and percent bonuses from every source, then combine.
    TMap<FName, float> Flat;
    TMap<FName, float> Percent;

    auto AccumulateModifier = [&Flat, &Percent](const FRLStatModifier& Mod, int32 Stacks)
    {
        if (Mod.AttributeName == NAME_None)
        {
            return;
        }
        Flat.FindOrAdd(Mod.AttributeName)    += Mod.FlatBonus    * Stacks;
        Percent.FindOrAdd(Mod.AttributeName) += Mod.PercentBonus * Stacks;
    };

    const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    const URLItemRegistry* Registry = GI ? GI->GetSubsystem<URLItemRegistry>() : nullptr;

    // Equipped gear: template base modifiers + rolled affixes.
    if (Inventory && Registry)
    {
        for (const TPair<ERLGearSlot, FGuid>& Slot : Inventory->GetEquipment())
        {
            FRLGearInstance GearInst;
            if (!Inventory->FindGear(Slot.Value, GearInst))
            {
                continue;
            }
            if (const URLGearDefinition* GearDef = Registry->GetGear(GearInst.GearId))
            {
                for (const FRLStatModifier& Mod : GearDef->BaseModifiers) { AccumulateModifier(Mod, 1); }
            }
            for (const FRLStatModifier& Mod : GearInst.RolledAffixes) { AccumulateModifier(Mod, 1); }
        }
    }

    // Active run boons (temporary power, stripped on return to base camp).
    if (GI && Registry)
    {
        if (const URLRunSubsystem* Run = GI->GetSubsystem<URLRunSubsystem>())
        {
            for (const FRLActiveBoon& Boon : Run->GetActiveBoons())
            {
                if (const URLBoonDefinition* BoonDef = Registry->GetBoon(Boon.BoonId))
                {
                    for (const FRLStatModifier& Mod : BoonDef->Modifiers) { AccumulateModifier(Mod, Boon.Stacks); }
                }
            }
        }
    }

    auto Combine = [&Flat, &Percent](float Base, const FName& Name)
    {
        return (Base + Flat.FindRef(Name)) * (1.f + Percent.FindRef(Name));
    };

    const float FinalMaxHealth = Combine(Stats.MaxHealth, TEXT("MaxHealth"));
    const float FinalMaxMana   = Combine(Stats.MaxMana,   TEXT("MaxMana"));

    Attr->SetMaxHealth(FinalMaxHealth);
    Attr->SetMaxMana(FinalMaxMana);
    Attr->SetHealthRegen(Combine(Stats.HealthRegen, TEXT("HealthRegen")));
    Attr->SetManaRegen(Combine(Stats.ManaRegen, TEXT("ManaRegen")));
    Attr->SetStrength(Combine(Stats.Strength, TEXT("Strength")));
    Attr->SetAgility(Combine(Stats.Agility, TEXT("Agility")));
    Attr->SetIntellect(Combine(Stats.Intellect, TEXT("Intellect")));
    Attr->SetCritChance(Combine(Stats.CritChance, TEXT("CritChance")));
    Attr->SetHaste(Combine(Stats.Haste, TEXT("Haste")));
    Attr->SetAdaptability(Combine(Stats.Adaptability, TEXT("Adaptability")));

    const float FinalMoveMult = Combine(1.f, TEXT("MoveSpeedMultiplier"));
    Attr->SetMoveSpeedMultiplier(FinalMoveMult);
    // Direct sets don't run PostGameplayEffectExecute, so push move speed onto
    // the character here (500 == the template's base MaxWalkSpeed).
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = 500.f * FMath::Max(0.01f, FinalMoveMult);
        }
    }

    if (bResetVitals)
    {
        Attr->SetHealth(FinalMaxHealth);
        Attr->SetMana(FinalMaxMana);
    }
    else
    {
        // Preserve current values, just re-clamp to the new maxima.
        Attr->SetHealth(FMath::Clamp(Attr->GetHealth(), 0.f, FinalMaxHealth));
        Attr->SetMana(FMath::Clamp(Attr->GetMana(), 0.f, FinalMaxMana));
    }

    // Derived offense scales off the class's primary (post-bonus) stat.
    const ERLPrimaryStat Scaling = Def ? Def->ScalingStat : ERLPrimaryStat::Strength;
    float PrimaryValue = Attr->GetStrength();
    switch (Scaling)
    {
    case ERLPrimaryStat::Agility:   PrimaryValue = Attr->GetAgility(); break;
    case ERLPrimaryStat::Intellect: PrimaryValue = Attr->GetIntellect(); break;
    default: break;
    }
    Attr->SetAttackPower(Combine(PrimaryValue * 2.f, TEXT("AttackPower")));
    Attr->SetSpellPower(Combine(Attr->GetIntellect() * 2.f, TEXT("SpellPower")));
}

float ARLPlayerState::GetExcessMana() const
{
    UAbilitySystemComponent* ASC = nullptr;
    if (const URLAttributeSet* Attr = ResolveAttributeSet(ASC))
    {
        return Attr->GetExcessMana();
    }
    return 0.f;
}

void ARLPlayerState::SetExcessMana(float Amount)
{
    UAbilitySystemComponent* ASC = nullptr;
    if (URLAttributeSet* Attr = ResolveAttributeSet(ASC))
    {
        Attr->SetExcessMana(FMath::Max(0.f, Amount));
    }
}

void ARLPlayerState::AddExcessMana(float Amount)
{
    if (Amount == 0.f)
    {
        return;
    }
    SetExcessMana(GetExcessMana() + Amount);
}

bool ARLPlayerState::SpendExcessMana(float Amount)
{
    if (Amount <= 0.f)
    {
        return true;
    }
    const float Current = GetExcessMana();
    if (Current < Amount)
    {
        return false;
    }
    SetExcessMana(Current - Amount);
    return true;
}

void ARLPlayerState::OnRep_Level()
{
    OnLevelChanged.Broadcast(Level, 0);
}

void ARLPlayerState::OnRep_Identity()
{
    OnLevelChanged.Broadcast(Level, 0);
}
