// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RLInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class URLInteractable : public UInterface
{
    GENERATED_BODY()
};

/**
 *  Something the player can walk up to and activate: altars, banking crates,
 *  the base-camp forge. The player controller line-traces for the nearest
 *  interactable and calls Interact on a button press.
 */
class IRLInteractable
{
    GENERATED_BODY()
public:
    /** Short prompt shown when the interactable is in focus (e.g. "Charge Altar"). */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
    FText GetInteractionPrompt() const;

    /** Whether the interactable can currently be used by this pawn. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
    bool CanInteract(APawn* InstigatorPawn) const;

    /** Perform the interaction. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
    void Interact(APawn* InstigatorPawn);
};
