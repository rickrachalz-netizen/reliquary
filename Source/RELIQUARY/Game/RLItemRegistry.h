// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RLTypes.h"
#include "RLItemRegistry.generated.h"

class URLGameData;
class URLMaterialDefinition;
class URLGearDefinition;
class URLRecipe;
class URLClassDefinition;
class URLBoonDefinition;

/**
 *  Loads the content library (URLGameData) once at startup and exposes fast
 *  id-based lookups for every definition. All other systems resolve ids
 *  through here, keeping saves as compact id lists.
 */
UCLASS()
class RELIQUARY_API URLItemRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    URLMaterialDefinition* GetMaterial(FName MaterialId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    URLGearDefinition* GetGear(FName GearId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    URLRecipe* GetRecipe(FName RecipeId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    URLClassDefinition* GetClassDefinition(ERLClass ClassId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    URLBoonDefinition* GetBoon(FName BoonId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    TArray<URLRecipe*> GetAllRecipes() const { return AllRecipes; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    TArray<URLBoonDefinition*> GetAllBoons() const { return AllBoons; }

    /** Materials that reliably spawn in a given zone, for route planning / spawners. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    TArray<URLMaterialDefinition*> GetMaterialsForZone(ERLZone Zone) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Registry")
    bool IsLoaded() const { return bLoaded; }

private:
    void BuildFrom(const URLGameData& Data);

    UPROPERTY() TObjectPtr<URLGameData> LoadedData;

    UPROPERTY() TMap<FName, TObjectPtr<URLMaterialDefinition>> MaterialsById;
    UPROPERTY() TMap<FName, TObjectPtr<URLGearDefinition>> GearById;
    UPROPERTY() TMap<FName, TObjectPtr<URLRecipe>> RecipesById;
    UPROPERTY() TMap<uint8, TObjectPtr<URLClassDefinition>> ClassesById;
    UPROPERTY() TMap<FName, TObjectPtr<URLBoonDefinition>> BoonsById;

    UPROPERTY() TArray<URLRecipe*> AllRecipes;
    UPROPERTY() TArray<URLBoonDefinition*> AllBoons;

    bool bLoaded = false;
};
