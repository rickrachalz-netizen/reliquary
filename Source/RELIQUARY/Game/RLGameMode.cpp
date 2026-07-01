// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLGameMode.h"
#include "RLPlayerState.h"
#include "RLProfileSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

ARLGameMode::ARLGameMode()
{
    // Persistent hero data rides on our custom PlayerState.
    PlayerStateClass = ARLPlayerState::StaticClass();
}

void ARLGameMode::BeginPlay()
{
    Super::BeginPlay();

    // On a base-camp / menu map, make sure the profile is loaded so the hero's
    // stats and stash are present. Run maps assume an already-loaded hero and an
    // active run started from base camp via the challenge altar.
    if (!bIsRunMap)
    {
        if (const UWorld* World = GetWorld())
        {
            if (UGameInstance* GI = World->GetGameInstance())
            {
                if (URLProfileSubsystem* Profile = GI->GetSubsystem<URLProfileSubsystem>())
                {
                    if (Profile->HasProfile())
                    {
                        Profile->LoadProfile();
                    }
                }
            }
        }
    }
}
