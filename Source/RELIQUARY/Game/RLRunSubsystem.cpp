// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLRunSubsystem.h"
#include "RLPlayerState.h"
#include "RLInventoryComponent.h"
#include "RLItemRegistry.h"
#include "RLBoonDefinition.h"
#include "RLDeveloperSettings.h"
#include "RLProfileSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void URLRunSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Phase = ERLRunPhase::InBaseCamp;
}

ARLPlayerState* URLRunSubsystem::GetPlayerState() const
{
    const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (!World)
    {
        return nullptr;
    }
    if (const APlayerController* PC = World->GetFirstPlayerController())
    {
        return PC->GetPlayerState<ARLPlayerState>();
    }
    return nullptr;
}

URLInventoryComponent* URLRunSubsystem::GetHomeInventory() const
{
    ARLPlayerState* PS = GetPlayerState();
    return PS ? PS->GetInventory() : nullptr;
}

URLItemRegistry* URLRunSubsystem::GetRegistry() const
{
    return GetGameInstance() ? GetGameInstance()->GetSubsystem<URLItemRegistry>() : nullptr;
}

void URLRunSubsystem::SetPhase(ERLRunPhase NewPhase)
{
    if (Phase != NewPhase)
    {
        Phase = NewPhase;
        OnRunPhaseChanged.Broadcast(Phase);

        // Autosave whenever the player reaches the safety of base camp.
        if (NewPhase == ERLRunPhase::InBaseCamp)
        {
            if (UGameInstance* GI = GetGameInstance())
            {
                if (URLProfileSubsystem* Profile = GI->GetSubsystem<URLProfileSubsystem>())
                {
                    Profile->SaveProfile();
                }
            }
        }
    }
}

float URLRunSubsystem::GetRunElapsedSeconds() const
{
    if (!IsRunActive())
    {
        return 0.f;
    }
    const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    const double Now = World ? World->GetTimeSeconds() : RunStartTime;
    return static_cast<float>(Now - RunStartTime);
}

float URLRunSubsystem::GetDifficultyScalar() const
{
    // Risk of Rain 2 in spirit: the world grows steadily more lethal with both
    // elapsed time and how deep the run has pushed.
    const float TimeFactor = GetRunElapsedSeconds() / 60.f;      // +1 per minute
    const float DepthFactor = static_cast<float>(FMath::Max(0, LevelIndex - 1));
    return 1.f + 0.15f * TimeFactor + 0.35f * DepthFactor;
}

void URLRunSubsystem::Embark(int32 InSeed, ERLZone StartingZone)
{
    // Roll and LOCK the seed at embark so the run can't be re-fished.
    RunSeed = (InSeed != 0) ? InSeed : FMath::Rand();
    LevelIndex = 1;
    MapsSinceBank = 0;
    CurrentZone = (StartingZone != ERLZone::None && StartingZone != ERLZone::BaseCamp)
                      ? StartingZone : ERLZone::WhisperingGrove;

    RunBag.Reset();
    ActiveBoons.Reset();

    if (ARLPlayerState* PS = GetPlayerState())
    {
        PS->SetExcessMana(0.f);
        PS->ApplyStatsToOwner(true); // fresh run: full vitals, no boons
    }

    const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    RunStartTime = World ? World->GetTimeSeconds() : 0.0;

    OnRunBagChanged.Broadcast();
    OnBoonsChanged.Broadcast();
    OnRunLevelChanged.Broadcast(LevelIndex);
    SetPhase(ERLRunPhase::Exploring);
}

void URLRunSubsystem::AddRunMaterial(FName MaterialId, int32 Count)
{
    if (MaterialId == NAME_None || Count <= 0 || !IsRunActive())
    {
        return;
    }
    if (FRLMaterialStack* Existing = RunBag.FindByPredicate(
            [MaterialId](const FRLMaterialStack& S) { return S.MaterialId == MaterialId; }))
    {
        Existing->Count += Count;
    }
    else
    {
        RunBag.Emplace(MaterialId, Count);
    }
    OnRunBagChanged.Broadcast();
}

void URLRunSubsystem::GrantExcessMana(float Amount)
{
    if (Amount <= 0.f || !IsRunActive())
    {
        return;
    }
    if (ARLPlayerState* PS = GetPlayerState())
    {
        PS->AddExcessMana(Amount);
    }
}

void URLRunSubsystem::OnChallengeAltarCleared()
{
    // Boss down and altar charged: the player now decides extract vs. onward.
    SetPhase(ERLRunPhase::Transition);
}

void URLRunSubsystem::DepositRunBagToHome()
{
    if (URLInventoryComponent* Home = GetHomeInventory())
    {
        Home->AddMaterials(RunBag);
    }
    RunBag.Reset();
    OnRunBagChanged.Broadcast();
}

void URLRunSubsystem::ClearRunPower()
{
    ActiveBoons.Reset();
    if (ARLPlayerState* PS = GetPlayerState())
    {
        PS->SetExcessMana(0.f);
        PS->ApplyStatsToOwner(true); // strip boons, refill vitals for base camp
    }
    OnBoonsChanged.Broadcast();
}

void URLRunSubsystem::ExtractToBaseCamp()
{
    // Safely home: keep everything gathered, award XP for the depth reached,
    // then strip all temporary run power.
    DepositRunBagToHome();

    if (ARLPlayerState* PS = GetPlayerState())
    {
        const URLDeveloperSettings* Settings = GetDefault<URLDeveloperSettings>();
        const int32 PerLevel = Settings ? Settings->BaseExperiencePerLevelCleared : 60;
        PS->GrantExperience(PerLevel * FMath::Max(1, LevelIndex));
        PS->GrantEssence(25 * FMath::Max(1, LevelIndex));
    }

    ClearRunPower();
    LevelIndex = 0;
    MapsSinceBank = 0;
    CurrentZone = ERLZone::BaseCamp;
    SetPhase(ERLRunPhase::InBaseCamp);
}

void URLRunSubsystem::TeleportOnward(ERLZone NextZone)
{
    const URLDeveloperSettings* Settings = GetDefault<URLDeveloperSettings>();
    const int32 MaxLevels = Settings ? Settings->LevelsPerRun : 10;
    const int32 BankEvery = Settings ? Settings->BankEveryNMaps : 3;

    if (LevelIndex >= MaxLevels)
    {
        // Reached the end of the path; the only way on is to extract.
        ExtractToBaseCamp();
        return;
    }

    ++LevelIndex;
    ++MapsSinceBank;
    CurrentZone = (NextZone != ERLZone::None) ? NextZone : CurrentZone;

    OnRunLevelChanged.Broadcast(LevelIndex);
    SetPhase(ERLRunPhase::Exploring);

    // Every N maps, offer a crate to bank progress without ending the run.
    if (BankEvery > 0 && MapsSinceBank >= BankEvery && RunBag.Num() > 0)
    {
        MapsSinceBank = 0;
        OnBankCrateOffered.Broadcast(LevelIndex);
    }
}

void URLRunSubsystem::UseBankCrate()
{
    // Send the current haul home but keep the run (and all run power) going.
    DepositRunBagToHome();
}

void URLRunSubsystem::OnPlayerDied()
{
    // Death forfeits every resource gathered on this run.
    RunBag.Reset();
    OnRunBagChanged.Broadcast();

    SetPhase(ERLRunPhase::Defeated);
    ClearRunPower();
    LevelIndex = 0;
    MapsSinceBank = 0;
    CurrentZone = ERLZone::BaseCamp;
    SetPhase(ERLRunPhase::InBaseCamp);
}

int32 URLRunSubsystem::GetBoonStacks(FName BoonId) const
{
    const FRLActiveBoon* Found = ActiveBoons.FindByPredicate(
        [BoonId](const FRLActiveBoon& B) { return B.BoonId == BoonId; });
    return Found ? Found->Stacks : 0;
}

int32 URLRunSubsystem::GetBoonCost(FName BoonId) const
{
    const URLItemRegistry* Registry = GetRegistry();
    const URLBoonDefinition* Def = Registry ? Registry->GetBoon(BoonId) : nullptr;
    if (!Def)
    {
        return 0;
    }
    return Def->GetCostForStack(GetBoonStacks(BoonId));
}

bool URLRunSubsystem::PurchaseBoon(FName BoonId)
{
    if (!IsRunActive())
    {
        return false;
    }
    const URLItemRegistry* Registry = GetRegistry();
    const URLBoonDefinition* Def = Registry ? Registry->GetBoon(BoonId) : nullptr;
    ARLPlayerState* PS = GetPlayerState();
    if (!Def || !PS)
    {
        return false;
    }

    const int32 Owned = GetBoonStacks(BoonId);
    if (Def->MaxStacks > 0 && Owned >= Def->MaxStacks)
    {
        return false;
    }

    const int32 Cost = Def->GetCostForStack(Owned);
    if (!PS->SpendExcessMana(static_cast<float>(Cost)))
    {
        return false;
    }

    if (FRLActiveBoon* Existing = ActiveBoons.FindByPredicate(
            [BoonId](const FRLActiveBoon& B) { return B.BoonId == BoonId; }))
    {
        ++Existing->Stacks;
    }
    else
    {
        FRLActiveBoon NewBoon;
        NewBoon.BoonId = BoonId;
        NewBoon.Stacks = 1;
        ActiveBoons.Add(NewBoon);
    }

    // Re-apply stats WITHOUT refilling vitals, so a boon doesn't heal you.
    PS->ApplyStatsToOwner(false);
    OnBoonsChanged.Broadcast();
    return true;
}

TArray<URLBoonDefinition*> URLRunSubsystem::RollBoonOffer(int32 AltarId, int32 Count) const
{
    TArray<URLBoonDefinition*> Offer;
    const URLItemRegistry* Registry = GetRegistry();
    if (!Registry)
    {
        return Offer;
    }

    TArray<URLBoonDefinition*> Pool = Registry->GetAllBoons();
    if (Pool.Num() == 0 || Count <= 0)
    {
        return Offer;
    }

    // Deterministic given the locked run seed + altar id: re-opening the same
    // altar always shows the same choices.
    const int32 StreamSeed = static_cast<int32>(static_cast<uint32>(RunSeed) ^ (static_cast<uint32>(AltarId) * 2654435761u));
    FRandomStream Stream(StreamSeed);
    const int32 Take = FMath::Min(Count, Pool.Num());
    for (int32 i = 0; i < Take; ++i)
    {
        const int32 Pick = Stream.RandRange(0, Pool.Num() - 1);
        Offer.Add(Pool[Pick]);
        Pool.RemoveAtSwap(Pick);
    }
    return Offer;
}
