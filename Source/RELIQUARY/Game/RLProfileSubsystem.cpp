// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLProfileSubsystem.h"
#include "RLSaveGame.h"
#include "RLPlayerState.h"
#include "RLInventoryComponent.h"
#include "RLDeveloperSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

FString URLProfileSubsystem::GetSlotName() const
{
    const URLDeveloperSettings* Settings = GetDefault<URLDeveloperSettings>();
    return (Settings && !Settings->SaveSlotName.IsEmpty()) ? Settings->SaveSlotName : TEXT("RELIQUARY_Profile");
}

ARLPlayerState* URLProfileSubsystem::GetPlayerState() const
{
    const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (!World)
    {
        return nullptr;
    }
    if (const APlayerController* PC = World->GetFirstPlayerController())
    {
        return PC->GetPlayerState<ARLPlayerState>();
    }
    return nullptr;
}

bool URLProfileSubsystem::HasProfile() const
{
    return UGameplayStatics::DoesSaveGameExist(GetSlotName(), 0);
}

URLSaveGame* URLProfileSubsystem::CaptureToSave() const
{
    ARLPlayerState* PS = GetPlayerState();
    if (!PS)
    {
        return nullptr;
    }

    URLSaveGame* Save = Cast<URLSaveGame>(UGameplayStatics::CreateSaveGameObject(URLSaveGame::StaticClass()));
    if (!Save)
    {
        return nullptr;
    }

    Save->HeroName = PS->HeroName;
    Save->CharacterClass = PS->CharacterClass;
    Save->ActiveSpec = PS->ActiveSpec;
    Save->Level = PS->Level;
    Save->Experience = PS->Experience;
    Save->UnspentTalentPoints = PS->UnspentTalentPoints;
    Save->EssenceLevel = PS->EssenceLevel;
    Save->EssenceProgress = PS->EssenceProgress;

    if (const URLInventoryComponent* Inv = PS->GetInventory())
    {
        Save->Materials = Inv->GetMaterials();
        Save->Gear = Inv->GetAllGear();
        Save->Equipped = Inv->GetEquipment();
        Save->KnownRecipes = Inv->GetKnownRecipeSet().Array();
    }
    return Save;
}

void URLProfileSubsystem::ApplyFromSave(const URLSaveGame& Save) const
{
    ARLPlayerState* PS = GetPlayerState();
    if (!PS)
    {
        return;
    }

    if (URLInventoryComponent* Inv = PS->GetInventory())
    {
        TSet<FName> Recipes(Save.KnownRecipes);
        Inv->RestoreState(Save.Materials, Save.Gear, Save.Equipped, Recipes);
    }

    // Apply identity/progression last so stat recompute sees the restored gear.
    PS->LoadFrom(Save.CharacterClass, Save.ActiveSpec, Save.Level, Save.Experience,
                 Save.UnspentTalentPoints, Save.EssenceLevel, Save.EssenceProgress, Save.HeroName);
}

void URLProfileSubsystem::CreateNewHero(const FString& HeroName, ERLClass InClass, ERLSpec InSpec)
{
    if (ARLPlayerState* PS = GetPlayerState())
    {
        if (URLInventoryComponent* Inv = PS->GetInventory())
        {
            Inv->ResetAll();
        }
        PS->HeroName = HeroName.IsEmpty() ? TEXT("Nameless Scavenger") : HeroName;
        PS->InitializeCharacter(InClass, InSpec);
    }
    SaveProfile();
}

bool URLProfileSubsystem::SaveProfile()
{
    URLSaveGame* Save = CaptureToSave();
    if (!Save)
    {
        return false;
    }
    const bool bOk = UGameplayStatics::SaveGameToSlot(Save, GetSlotName(), 0);
    if (bOk)
    {
        OnProfileSaved.Broadcast();
    }
    return bOk;
}

bool URLProfileSubsystem::LoadProfile()
{
    if (!HasProfile())
    {
        return false;
    }
    URLSaveGame* Save = Cast<URLSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), 0));
    if (!Save)
    {
        return false;
    }
    ApplyFromSave(*Save);
    OnProfileLoaded.Broadcast();
    return true;
}
