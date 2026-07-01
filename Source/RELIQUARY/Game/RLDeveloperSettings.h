// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RLDeveloperSettings.generated.h"

class URLGameData;

/**
 *  Project settings for RELIQUARY, editable under Project Settings > Game >
 *  "RELIQUARY" and persisted to DefaultGame.ini. Points the runtime at the
 *  content library and holds a few global tuning knobs.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "RELIQUARY"))
class RELIQUARY_API URLDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    virtual FName GetCategoryName() const override { return TEXT("Game"); }

    /** The content library loaded by URLItemRegistry at startup. */
    UPROPERTY(config, EditAnywhere, Category = "Content", meta = (AllowedClasses = "/Script/RELIQUARY.RLGameData"))
    TSoftObjectPtr<URLGameData> GameData;

    /** Number of interconnected levels in a full run before the end boss. */
    UPROPERTY(config, EditAnywhere, Category = "Run", meta = (ClampMin = 1))
    int32 LevelsPerRun = 10;

    /** The player is offered a banking crate every N maps. */
    UPROPERTY(config, EditAnywhere, Category = "Run", meta = (ClampMin = 1))
    int32 BankEveryNMaps = 3;

    /** Base XP awarded per level cleared on a successful extraction. */
    UPROPERTY(config, EditAnywhere, Category = "Run", meta = (ClampMin = 0))
    int32 BaseExperiencePerLevelCleared = 60;

    /** SaveGame slot used for the persistent profile. */
    UPROPERTY(config, EditAnywhere, Category = "Save")
    FString SaveSlotName = TEXT("RELIQUARY_Profile");
};
