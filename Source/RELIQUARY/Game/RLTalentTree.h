// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RLTypes.h"
#include "RLTalentTree.generated.h"

/**
 *  A single node in a spec's talent tree. Talents are deliberately simple and
 *  legible: a node grants flat/percent stat changes and/or a tagged gameplay
 *  hook that abilities read. Multiple viable builds emerge from which nodes a
 *  player invests into and in what order.
 */
USTRUCT(BlueprintType)
struct FRLTalentNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    FName NodeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent", meta = (MultiLine = true))
    FText Description;

    /** Row/column in the tree, for UI layout. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    int32 Row = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    int32 Column = 0;

    /** Talent points that must already be spent in this tree before this unlocks. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    int32 RequiredPointsInTree = 0;

    /** Maximum ranks a player can put into this node. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent", meta = (ClampMin = 1))
    int32 MaxRank = 1;

    /** Node ids that must be taken first. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    TArray<FName> Prerequisites;

    /**
     *  Gameplay tag this node contributes when taken (e.g. "Talent.Warrior.Cleave").
     *  Abilities and passives query the owner's granted talent tags to change
     *  behaviour without any bespoke wiring.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    FGameplayTag GrantedTag;

    /** Optional flat attribute grants per rank, keyed by attribute name. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent")
    TMap<FName, float> FlatStatPerRank;
};

/**
 *  The full talent tree for one spec, authored as a data asset.
 */
UCLASS(BlueprintType)
class RELIQUARY_API URLTalentTree : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLClass OwningClass = ERLClass::Warrior;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    ERLSpec Spec = ERLSpec::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talents")
    TArray<FRLTalentNode> Nodes;

    /** Find a node by id (nullptr if absent). */
    const FRLTalentNode* FindNode(FName NodeId) const
    {
        return Nodes.FindByPredicate([NodeId](const FRLTalentNode& N) { return N.NodeId == NodeId; });
    }
};
