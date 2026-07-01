// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLInteractable.h"
#include "RLTypes.h"
#include "RLChallengeAltar.generated.h"

class UStaticMeshComponent;
class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRLOnAltarChargeChanged, float, ChargePct);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRLOnAltarReady);

/**
 *  A challenge altar: the gateway between the realm's ten levels. Activating it
 *  empowers and spawns a boss; the player must charge the altar by standing in
 *  its radius while the boss lives. Once charged AND the boss is dead, the
 *  player interacts to either extract to base camp or teleport onward.
 */
UCLASS()
class RELIQUARY_API ARLChallengeAltar : public AActor, public IRLInteractable
{
    GENERATED_BODY()
public:
    ARLChallengeAltar();

    /** Boss to spawn on activation. Left to content to avoid hard asset refs in C++. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altar")
    TSubclassOf<AActor> BossClass;

    /** Where the destination of "teleport onward" leads, for route planning. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altar")
    ERLZone NextZone = ERLZone::None;

    /** Seconds of in-radius standing required to fully charge (scaled by boss life). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altar", meta = (ClampMin = 1))
    float ChargeDuration = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altar", meta = (ClampMin = 100))
    float ChargeRadius = 600.f;

    UPROPERTY(BlueprintAssignable, Category = "Altar")
    FRLOnAltarChargeChanged OnChargeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Altar")
    FRLOnAltarReady OnAltarReady;

    /** Begin the challenge: spawn the boss and start accepting charge. */
    UFUNCTION(BlueprintCallable, Category = "Altar")
    void ActivateChallenge(APawn* InstigatorPawn);

    /** Called by the spawned boss (or its death delegate) when it dies. */
    UFUNCTION(BlueprintCallable, Category = "Altar")
    void NotifyBossDefeated();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Altar")
    bool IsReady() const { return bCharged && bBossDefeated; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Altar")
    float GetChargePct() const { return ChargeDuration > 0.f ? Charge / ChargeDuration : 1.f; }

    /** Resolve the altar as an extraction: end the run and go home. */
    UFUNCTION(BlueprintCallable, Category = "Altar")
    void ChooseExtract();

    /** Resolve the altar as a push deeper: teleport onward to a fresh map. */
    UFUNCTION(BlueprintCallable, Category = "Altar")
    void ChooseOnward();

    // IRLInteractable
    virtual FText GetInteractionPrompt_Implementation() const override;
    virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;

protected:
    virtual void Tick(float DeltaSeconds) override;

    /** Content hook to spawn/scale VFX for the active altar beam. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Altar")
    void OnActivated();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> ChargeVolume;

    /** True once the player has begun (and only then does charge accrue). */
    UPROPERTY(BlueprintReadOnly, Category = "Altar")
    bool bActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Altar")
    bool bCharged = false;

    UPROPERTY(BlueprintReadOnly, Category = "Altar")
    bool bBossDefeated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Altar")
    float Charge = 0.f;

    UPROPERTY() TObjectPtr<AActor> SpawnedBoss;

    bool IsAnyPlayerInRadius() const;
};
