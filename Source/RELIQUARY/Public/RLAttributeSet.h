// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RLAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 *  RELIQUARY attribute set.
 *
 *  Implements the wide, expressive stat set called for in the design doc:
 *   - Vitals:    Health / MaxHealth, Mana / MaxMana, and their regen rates.
 *   - Primary:   Strength, Agility, Intellect (the three class-defining stats).
 *   - Secondary: CritChance, Haste, and the signature "Adaptability" stat.
 *   - Offense:   AttackPower / SpellPower (derived, recomputed from primaries).
 *   - Utility:   MoveSpeedMultiplier.
 *   - Run:       ExcessMana, the temporary currency spent at power altars.
 *   - Meta:      IncomingDamage / IncomingHealing (transient, never replicated),
 *                the channel GameplayEffects use to modify Health.
 */
UCLASS()
class RELIQUARY_API URLAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    URLAttributeSet();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    // ---- Vitals ---------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vitals")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, Health)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vitals")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, MaxHealth)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegen, Category = "Vitals")
    FGameplayAttributeData HealthRegen;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, HealthRegen)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vitals")
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, Mana)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vitals")
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, MaxMana)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegen, Category = "Vitals")
    FGameplayAttributeData ManaRegen;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, ManaRegen)

    // ---- Primary stats --------------------------------------------------

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary")
    FGameplayAttributeData Strength;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, Strength)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility, Category = "Primary")
    FGameplayAttributeData Agility;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, Agility)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intellect, Category = "Primary")
    FGameplayAttributeData Intellect;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, Intellect)

    // ---- Secondary stats ------------------------------------------------

    /** Chance (0..1) for attacks and abilities to critically strike. */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritChance, Category = "Secondary")
    FGameplayAttributeData CritChance;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, CritChance)

    /** Reduces ability cooldowns and increases attack/cast speed. 1.0 == baseline. */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Haste, Category = "Secondary")
    FGameplayAttributeData Haste;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, Haste)

    /**
     *  Adaptability (signature stat): increases the damage of abilities and
     *  attacks by this % when they aren't a repeat of the last action,
     *  stacking up to five times. Stored here as the per-stack magnitude;
     *  the combat code applies and stacks it.
     */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Adaptability, Category = "Secondary")
    FGameplayAttributeData Adaptability;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, Adaptability)

    // ---- Derived offense ------------------------------------------------

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Offense")
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, AttackPower)

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SpellPower, Category = "Offense")
    FGameplayAttributeData SpellPower;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, SpellPower)

    // ---- Utility --------------------------------------------------------

    /** Multiplier applied to base movement speed. 1.0 == baseline. */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeedMultiplier, Category = "Utility")
    FGameplayAttributeData MoveSpeedMultiplier;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, MoveSpeedMultiplier)

    // ---- Run currency ---------------------------------------------------

    /** Temporary run currency spent at power altars. Reset to 0 on return to base camp. */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ExcessMana, Category = "Run")
    FGameplayAttributeData ExcessMana;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, ExcessMana)

    // ---- Meta attributes (transient, not replicated) --------------------

    /** Damage to apply this frame. Consumed in PostGameplayEffectExecute -> Health. */
    UPROPERTY(BlueprintReadOnly, Category = "Meta")
    FGameplayAttributeData IncomingDamage;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, IncomingDamage)

    /** Healing to apply this frame. Consumed in PostGameplayEffectExecute -> Health. */
    UPROPERTY(BlueprintReadOnly, Category = "Meta")
    FGameplayAttributeData IncomingHealing;
    ATTRIBUTE_ACCESSORS(URLAttributeSet, IncomingHealing)

protected:
    /** Clamp a current vital when its maximum changes so ratios are preserved sensibly. */
    void AdjustAttributeForMaxChange(const FGameplayAttributeData& AffectedAttribute,
                                     const FGameplayAttributeData& MaxAttribute,
                                     float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const;

    UFUNCTION() void OnRep_Health(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_HealthRegen(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_Mana(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_MaxMana(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_ManaRegen(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_Strength(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_Agility(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_Intellect(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_CritChance(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_Haste(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_Adaptability(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_AttackPower(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_SpellPower(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& Old);
    UFUNCTION() void OnRep_ExcessMana(const FGameplayAttributeData& Old);
};
