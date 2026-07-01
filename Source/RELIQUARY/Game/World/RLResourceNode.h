// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatDamageable.h"
#include "RLTypes.h"
#include "RLResourceNode.generated.h"

class UStaticMeshComponent;
class ARLResourceNode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRLOnNodeHarvested, ARLResourceNode*, Node);

/**
 *  A destructible, harvestable resource node — the oak trees, ironwood, and
 *  stone outcrops that make the environment itself a source of materials.
 *
 *  Because attacks and abilities damage anything implementing ICombatDamageable,
 *  the existing combat already shatters these; each hit chips the node and, when
 *  it breaks, its distinct material drops into the run bag and showers the
 *  player with a little excess mana.
 */
UCLASS()
class RELIQUARY_API ARLResourceNode : public AActor, public ICombatDamageable
{
    GENERATED_BODY()
public:
    ARLResourceNode();

    /** The distinct material this node yields (e.g. "Mat.Ironwood"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    FName MaterialId = NAME_None;

    /** Family, used for spawn-time validation and generic effects. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    ERLMaterialFamily Family = ERLMaterialFamily::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = 1))
    int32 MinYield = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = 1))
    int32 MaxYield = 3;

    /** Hit points; each attack subtracts its damage until the node breaks. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = 1))
    float MaxHealth = 20.f;

    /** Excess mana granted to the harvester when the node breaks. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = 0))
    float ExcessManaOnBreak = 10.f;

    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FRLOnNodeHarvested OnHarvested;

    // ICombatDamageable
    virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;
    virtual void HandleDeath() override;
    virtual void ApplyHealing(float Healing, AActor* Healer) override;
    virtual void NotifyDanger(const FVector& DangerLocation, AActor* DangerSource) override;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Resource")
    float GetCurrentHealth() const { return CurrentHealth; }

protected:
    virtual void BeginPlay() override;

    /** Blueprint hook to play a hit reaction (shake, chips, sound). */
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource")
    void OnHit(const FVector& HitLocation, float Damage);

    /** Blueprint hook to spawn break VFX / debris before the actor is destroyed. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource")
    void OnBreak(const FVector& BreakLocation);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
    float CurrentHealth = 0.f;

    /** Award the harvested material to the run bag and mana to the harvester. */
    void GrantHarvest(AActor* Harvester);

    bool bHarvested = false;
};
