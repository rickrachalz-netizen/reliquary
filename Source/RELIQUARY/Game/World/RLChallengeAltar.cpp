// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLChallengeAltar.h"
#include "RLRunSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

ARLChallengeAltar::ARLChallengeAltar()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false; // only tick while active

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    ChargeVolume = CreateDefaultSubobject<USphereComponent>(TEXT("ChargeVolume"));
    ChargeVolume->SetupAttachment(Mesh);
    ChargeVolume->SetSphereRadius(ChargeRadius);
    ChargeVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ChargeVolume->SetCollisionResponseToAllChannels(ECR_Overlap);
    ChargeVolume->SetGenerateOverlapEvents(true);
}

static URLRunSubsystem* GetRun(const AActor* Actor)
{
    const UWorld* World = Actor ? Actor->GetWorld() : nullptr;
    UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
    return GI ? GI->GetSubsystem<URLRunSubsystem>() : nullptr;
}

void ARLChallengeAltar::ActivateChallenge(APawn* InstigatorPawn)
{
    if (bActive)
    {
        return;
    }
    bActive = true;
    bCharged = false;
    bBossDefeated = false;
    Charge = 0.f;

    ChargeVolume->SetSphereRadius(ChargeRadius);

    // The empowered boss can read URLRunSubsystem::GetDifficultyScalar() to scale
    // its own stats on spawn.
    if (BossClass && GetWorld())
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        SpawnedBoss = GetWorld()->SpawnActor<AActor>(BossClass, GetActorLocation() + FVector(300.f, 0.f, 0.f), GetActorRotation(), Params);
    }

    SetActorTickEnabled(true);
    OnActivated();
}

bool ARLChallengeAltar::IsAnyPlayerInRadius() const
{
    TArray<AActor*> Overlapping;
    ChargeVolume->GetOverlappingActors(Overlapping, APawn::StaticClass());
    for (const AActor* Actor : Overlapping)
    {
        if (const APawn* Pawn = Cast<APawn>(Actor))
        {
            if (Pawn->IsPlayerControlled())
            {
                return true;
            }
        }
    }
    return false;
}

void ARLChallengeAltar::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bActive || bCharged)
    {
        return;
    }

    // Charge only accrues while a player stands in the radius.
    if (IsAnyPlayerInRadius())
    {
        Charge = FMath::Min(Charge + DeltaSeconds, ChargeDuration);
        OnChargeChanged.Broadcast(GetChargePct());

        if (Charge >= ChargeDuration)
        {
            bCharged = true;
            if (IsReady())
            {
                OnAltarReady.Broadcast();
            }
        }
    }
}

void ARLChallengeAltar::NotifyBossDefeated()
{
    bBossDefeated = true;
    SpawnedBoss = nullptr;

    if (bCharged)
    {
        OnAltarReady.Broadcast();
    }

    if (URLRunSubsystem* Run = GetRun(this))
    {
        Run->OnChallengeAltarCleared();
    }
}

void ARLChallengeAltar::ChooseExtract()
{
    if (!IsReady())
    {
        return;
    }
    if (URLRunSubsystem* Run = GetRun(this))
    {
        Run->ExtractToBaseCamp();
    }
    SetActorTickEnabled(false);
    bActive = false;
}

void ARLChallengeAltar::ChooseOnward()
{
    if (!IsReady())
    {
        return;
    }
    if (URLRunSubsystem* Run = GetRun(this))
    {
        Run->TeleportOnward(NextZone);
    }
    SetActorTickEnabled(false);
    bActive = false;
}

FText ARLChallengeAltar::GetInteractionPrompt_Implementation() const
{
    if (!bActive)
    {
        return NSLOCTEXT("RL", "AltarChallenge", "Activate Challenge Altar");
    }
    if (IsReady())
    {
        return NSLOCTEXT("RL", "AltarResolve", "Extract or Travel Onward");
    }
    return NSLOCTEXT("RL", "AltarCharging", "Defend the Altar");
}

bool ARLChallengeAltar::CanInteract_Implementation(APawn* /*InstigatorPawn*/) const
{
    // Interactable to start the challenge, or to resolve it once ready.
    return !bActive || IsReady();
}

void ARLChallengeAltar::Interact_Implementation(APawn* InstigatorPawn)
{
    if (!bActive)
    {
        ActivateChallenge(InstigatorPawn);
    }
    // When ready, the extract/onward choice is a UI decision routed to
    // ChooseExtract / ChooseOnward, so plain interaction does nothing here.
}
