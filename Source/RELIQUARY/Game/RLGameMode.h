// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RELIQUARYGameMode.h"
#include "RLTypes.h"
#include "RLGameMode.generated.h"

/**
 *  Concrete game mode for RELIQUARY levels. Installs the persistent hero
 *  PlayerState and, on a run map, kicks off (or resumes) the run once the
 *  player is ready. Base-camp and run maps can share this mode and branch on
 *  bIsRunMap.
 */
UCLASS()
class RELIQUARY_API ARLGameMode : public ARELIQUARYGameMode
{
    GENERATED_BODY()
public:
    ARLGameMode();

    /** True on procedural run maps; false in base camp / menus. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RELIQUARY")
    bool bIsRunMap = false;

    /** Zone this map represents (drives which materials spawn and route planning). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RELIQUARY")
    ERLZone Zone = ERLZone::BaseCamp;

protected:
    virtual void BeginPlay() override;
};
