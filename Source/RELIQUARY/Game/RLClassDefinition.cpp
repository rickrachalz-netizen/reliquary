// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLClassDefinition.h"

FRLBaseStats URLClassDefinition::GetStatsForLevel(int32 Level) const
{
    const int32 Steps = FMath::Max(0, Level - 1);

    FRLBaseStats Out = BaseStats;
    Out.MaxHealth    += PerLevelGrowth.MaxHealth    * Steps;
    Out.MaxMana      += PerLevelGrowth.MaxMana      * Steps;
    Out.HealthRegen  += PerLevelGrowth.HealthRegen  * Steps;
    Out.ManaRegen    += PerLevelGrowth.ManaRegen    * Steps;
    Out.Strength     += PerLevelGrowth.Strength     * Steps;
    Out.Agility      += PerLevelGrowth.Agility      * Steps;
    Out.Intellect    += PerLevelGrowth.Intellect    * Steps;
    Out.CritChance   += PerLevelGrowth.CritChance   * Steps;
    Out.Haste        += PerLevelGrowth.Haste        * Steps;
    Out.Adaptability += PerLevelGrowth.Adaptability * Steps;
    return Out;
}
