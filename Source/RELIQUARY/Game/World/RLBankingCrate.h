// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLInteractable.h"
#include "RLBankingCrate.generated.h"

class UStaticMeshComponent;

/**
 *  A banking crate, offered every few maps. Interacting sends the current run
 *  bag home to base camp while the run continues — locking in progress without
 *  giving up run power. Single use.
 */
UCLASS()
class RELIQUARY_API ARLBankingCrate : public AActor, public IRLInteractable
{
    GENERATED_BODY()
public:
    ARLBankingCrate();

    UPROPERTY(BlueprintReadOnly, Category = "Crate")
    bool bUsed = false;

    // IRLInteractable
    virtual FText GetInteractionPrompt_Implementation() const override;
    virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;

protected:
    /** Content hook for the "resources shipped home" flourish. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Crate")
    void OnBanked();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;
};
