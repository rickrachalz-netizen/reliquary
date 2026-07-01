// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLItemRegistry.h"
#include "RLGameData.h"
#include "RLDeveloperSettings.h"
#include "RLMaterialDefinition.h"
#include "RLGearDefinition.h"
#include "RLRecipe.h"
#include "RLClassDefinition.h"
#include "RLBoonDefinition.h"

void URLItemRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const URLDeveloperSettings* Settings = GetDefault<URLDeveloperSettings>();
    if (!Settings)
    {
        return;
    }

    // Synchronously load the content library. It is small (definitions only)
    // and needed before the first character or run is created.
    if (URLGameData* Data = Settings->GameData.LoadSynchronous())
    {
        LoadedData = Data;
        BuildFrom(*Data);
        bLoaded = true;
    }
}

void URLItemRegistry::Deinitialize()
{
    MaterialsById.Reset();
    GearById.Reset();
    RecipesById.Reset();
    ClassesById.Reset();
    BoonsById.Reset();
    AllRecipes.Reset();
    AllBoons.Reset();
    LoadedData = nullptr;
    bLoaded = false;
    Super::Deinitialize();
}

void URLItemRegistry::BuildFrom(const URLGameData& Data)
{
    for (URLMaterialDefinition* Mat : Data.Materials)
    {
        if (Mat && Mat->MaterialId != NAME_None)
        {
            MaterialsById.Add(Mat->MaterialId, Mat);
        }
    }
    for (URLGearDefinition* G : Data.Gear)
    {
        if (G && G->GearId != NAME_None)
        {
            GearById.Add(G->GearId, G);
        }
    }
    for (URLRecipe* R : Data.Recipes)
    {
        if (R && R->RecipeId != NAME_None)
        {
            RecipesById.Add(R->RecipeId, R);
            AllRecipes.Add(R);
        }
    }
    for (URLClassDefinition* C : Data.Classes)
    {
        if (C)
        {
            ClassesById.Add(static_cast<uint8>(C->ClassId), C);
        }
    }
    for (URLBoonDefinition* B : Data.Boons)
    {
        if (B && B->BoonId != NAME_None)
        {
            BoonsById.Add(B->BoonId, B);
            AllBoons.Add(B);
        }
    }
}

URLMaterialDefinition* URLItemRegistry::GetMaterial(FName MaterialId) const
{
    const TObjectPtr<URLMaterialDefinition>* Found = MaterialsById.Find(MaterialId);
    return Found ? *Found : nullptr;
}

URLGearDefinition* URLItemRegistry::GetGear(FName GearId) const
{
    const TObjectPtr<URLGearDefinition>* Found = GearById.Find(GearId);
    return Found ? *Found : nullptr;
}

URLRecipe* URLItemRegistry::GetRecipe(FName RecipeId) const
{
    const TObjectPtr<URLRecipe>* Found = RecipesById.Find(RecipeId);
    return Found ? *Found : nullptr;
}

URLClassDefinition* URLItemRegistry::GetClassDefinition(ERLClass ClassId) const
{
    const TObjectPtr<URLClassDefinition>* Found = ClassesById.Find(static_cast<uint8>(ClassId));
    return Found ? *Found : nullptr;
}

URLBoonDefinition* URLItemRegistry::GetBoon(FName BoonId) const
{
    const TObjectPtr<URLBoonDefinition>* Found = BoonsById.Find(BoonId);
    return Found ? *Found : nullptr;
}

TArray<URLMaterialDefinition*> URLItemRegistry::GetMaterialsForZone(ERLZone Zone) const
{
    TArray<URLMaterialDefinition*> Result;
    for (const TPair<FName, TObjectPtr<URLMaterialDefinition>>& Pair : MaterialsById)
    {
        if (Pair.Value && Pair.Value->SourceZones.Contains(Zone))
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}
