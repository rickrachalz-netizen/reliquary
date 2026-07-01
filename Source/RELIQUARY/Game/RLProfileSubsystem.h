// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RLTypes.h"
#include "RLProfileSubsystem.generated.h"

class ARLPlayerState;
class URLSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRLOnProfileSaved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRLOnProfileLoaded);

/**
 *  Owns the persistent profile: creating a new hero, saving on entering base
 *  camp (autosave), and loading on boot. Deliberately the only place that
 *  touches the save slot, so save policy stays in one spot.
 */
UCLASS()
class RELIQUARY_API URLProfileSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, Category = "Profile") FRLOnProfileSaved  OnProfileSaved;
    UPROPERTY(BlueprintAssignable, Category = "Profile") FRLOnProfileLoaded OnProfileLoaded;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profile")
    bool HasProfile() const;

    /** Create a brand-new hero of the given class/spec and persist it. */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    void CreateNewHero(const FString& HeroName, ERLClass InClass, ERLSpec InSpec);

    /** Write the current PlayerState + inventory to the save slot. Autosave entry point. */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool SaveProfile();

    /** Load the save slot and apply it to the current PlayerState + inventory. */
    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool LoadProfile();

private:
    FString GetSlotName() const;
    ARLPlayerState* GetPlayerState() const;

    /** Snapshot the live game state into a save object. */
    URLSaveGame* CaptureToSave() const;

    /** Apply a save object onto the live PlayerState + inventory. */
    void ApplyFromSave(const URLSaveGame& Save) const;
};
