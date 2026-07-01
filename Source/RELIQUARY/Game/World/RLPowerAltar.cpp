// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLPowerAltar.h"
#include "RLRunSubsystem.h"
#include "RLBoonDefinition.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

ARLPowerAltar::ARLPowerAltar()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
}

static URLRunSubsystem* GetRunSubsystem(const AActor* Actor)
{
    const UWorld* World = Actor ? Actor->GetWorld() : nullptr;
    UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
    return GI ? GI->GetSubsystem<URLRunSubsystem>() : nullptr;
}

TArray<URLBoonDefinition*> ARLPowerAltar::GetOffer() const
{
    if (URLRunSubsystem* Run = GetRunSubsystem(this))
    {
        return Run->RollBoonOffer(AltarId, OfferCount);
    }
    return {};
}

bool ARLPowerAltar::PurchaseFromOffer(FName BoonId)
{
    if (bConsumed)
    {
        return false;
    }

    // Only allow buying a boon that is actually in this altar's offer.
    const TArray<URLBoonDefinition*> Offer = GetOffer();
    const bool bInOffer = Offer.ContainsByPredicate(
        [BoonId](const URLBoonDefinition* B) { return B && B->BoonId == BoonId; });
    if (!bInOffer)
    {
        return false;
    }

    URLRunSubsystem* Run = GetRunSubsystem(this);
    if (Run && Run->PurchaseBoon(BoonId))
    {
        bConsumed = true;
        return true;
    }
    return false;
}

FText ARLPowerAltar::GetInteractionPrompt_Implementation() const
{
    return bConsumed
        ? NSLOCTEXT("RL", "AltarSpent", "Spent")
        : NSLOCTEXT("RL", "AltarBoon", "Offer Excess Mana");
}

bool ARLPowerAltar::CanInteract_Implementation(APawn* /*InstigatorPawn*/) const
{
    return !bConsumed;
}

void ARLPowerAltar::Interact_Implementation(APawn* /*InstigatorPawn*/)
{
    if (bConsumed)
    {
        return;
    }
    OnRequestBoonChoice(GetOffer());
}
