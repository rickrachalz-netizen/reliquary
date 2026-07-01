// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLBankingCrate.h"
#include "RLRunSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

ARLBankingCrate::ARLBankingCrate()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
}

FText ARLBankingCrate::GetInteractionPrompt_Implementation() const
{
    return bUsed
        ? NSLOCTEXT("RL", "CrateUsed", "Empty")
        : NSLOCTEXT("RL", "CrateBank", "Ship Resources Home");
}

bool ARLBankingCrate::CanInteract_Implementation(APawn* /*InstigatorPawn*/) const
{
    return !bUsed;
}

void ARLBankingCrate::Interact_Implementation(APawn* /*InstigatorPawn*/)
{
    if (bUsed)
    {
        return;
    }

    const UWorld* World = GetWorld();
    UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
    if (URLRunSubsystem* Run = GI ? GI->GetSubsystem<URLRunSubsystem>() : nullptr)
    {
        Run->UseBankCrate();
        bUsed = true;
        OnBanked();
    }
}
