// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RLGameData.generated.h"

class URLMaterialDefinition;
class URLGearDefinition;
class URLRecipe;
class URLClassDefinition;
class URLBoonDefinition;

/**
 *  The single content library the game loads at startup. Bundling every
 *  definition here (rather than scanning the asset registry) keeps runtime
 *  lookups cheap and makes the game's whole data surface reviewable in one
 *  asset. Referenced from URLDeveloperSettings.
 */
UCLASS(BlueprintType)
class RELIQUARY_API URLGameData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    TArray<TObjectPtr<URLClassDefinition>> Classes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    TArray<TObjectPtr<URLMaterialDefinition>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    TArray<TObjectPtr<URLGearDefinition>> Gear;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    TArray<TObjectPtr<URLRecipe>> Recipes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Content")
    TArray<TObjectPtr<URLBoonDefinition>> Boons;
};
