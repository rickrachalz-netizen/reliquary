// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RLInteractable.h"
#include "RLPowerAltar.generated.h"

class UStaticMeshComponent;
class URLBoonDefinition;

/**
 *  A power altar: spend excess mana to choose from a small set of offered
 *  boons, each pushing power in a different direction. Only a set number appear
 *  per level. The offer is deterministic given the locked run seed + this
 *  altar's id, so the choice can't be re-rolled by leaving and returning.
 */
UCLASS()
class RELIQUARY_API ARLPowerAltar : public AActor, public IRLInteractable
{
    GENERATED_BODY()
public:
    ARLPowerAltar();

    /** Distinct id per placed altar; drives the deterministic offer. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altar")
    int32 AltarId = 0;

    /** How many boons to present. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altar", meta = (ClampMin = 1, ClampMax = 5))
    int32 OfferCount = 3;

    /** Whether this altar has already been spent (one purchase per altar). */
    UPROPERTY(BlueprintReadOnly, Category = "Altar")
    bool bConsumed = false;

    /** Return the (deterministic) set of boons this altar offers. */
    UFUNCTION(BlueprintCallable, Category = "Altar")
    TArray<URLBoonDefinition*> GetOffer() const;

    /** Buy one of the offered boons by id; consumes the altar on success. */
    UFUNCTION(BlueprintCallable, Category = "Altar")
    bool PurchaseFromOffer(FName BoonId);

    // IRLInteractable
    virtual FText GetInteractionPrompt_Implementation() const override;
    virtual bool CanInteract_Implementation(APawn* InstigatorPawn) const override;
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;

protected:
    /** Content hook to open the boon-selection UI for this altar. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Altar")
    void OnRequestBoonChoice(const TArray<URLBoonDefinition*>& Offer);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;
};
