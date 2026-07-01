// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLResourceNode.h"
#include "RLRunSubsystem.h"
#include "RLPlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

ARLResourceNode::ARLResourceNode()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    // The combat attack sweep queries ECC_Pawn and ECC_WorldDynamic *object types*,
    // so the node must present as WorldDynamic to be hit by melee/abilities.
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    // Let pawns walk right up to it without being physically blocked.
    Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARLResourceNode::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    if (MaxYield < MinYield)
    {
        MaxYield = MinYield;
    }
}

void ARLResourceNode::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& /*DamageImpulse*/)
{
    if (bHarvested || Damage <= 0.f)
    {
        return;
    }

    CurrentHealth -= Damage;
    OnHit(DamageLocation, Damage);

    if (CurrentHealth <= 0.f)
    {
        GrantHarvest(DamageCauser);
        HandleDeath();
    }
}

void ARLResourceNode::GrantHarvest(AActor* Harvester)
{
    if (bHarvested)
    {
        return;
    }
    bHarvested = true;

    const int32 Yield = FMath::RandRange(MinYield, MaxYield);

    if (const UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (URLRunSubsystem* Run = GI->GetSubsystem<URLRunSubsystem>())
            {
                Run->AddRunMaterial(MaterialId, Yield);
                Run->GrantExcessMana(ExcessManaOnBreak);
            }
        }
    }

    OnHarvested.Broadcast(this);
}

void ARLResourceNode::HandleDeath()
{
    OnBreak(GetActorLocation());
    Destroy();
}

void ARLResourceNode::ApplyHealing(float /*Healing*/, AActor* /*Healer*/)
{
    // Resource nodes don't heal.
}

void ARLResourceNode::NotifyDanger(const FVector& /*DangerLocation*/, AActor* /*DangerSource*/)
{
    // Inanimate; nothing to react to.
}
