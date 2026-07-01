// Copyright Epic Games, Inc. All Rights Reserved.

#include "RLAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URLAttributeSet::URLAttributeSet()
{
    // Sensible baselines. Real starting values are pushed in by the class
    // definition / progression system when a character is created or levels up.
    InitMaxHealth(100.f);
    InitHealth(100.f);
    InitMaxMana(100.f);
    InitMana(100.f);
    InitHealthRegen(1.f);
    InitManaRegen(5.f);

    InitStrength(10.f);
    InitAgility(10.f);
    InitIntellect(10.f);

    InitCritChance(0.05f);
    InitHaste(1.f);
    InitAdaptability(0.02f);

    InitAttackPower(0.f);
    InitSpellPower(0.f);
    InitMoveSpeedMultiplier(1.f);
    InitExcessMana(0.f);
}

void URLAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, ManaRegen, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, Strength, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, Agility, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, Intellect, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, CritChance, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, Haste, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, Adaptability, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, SpellPower, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, MoveSpeedMultiplier, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URLAttributeSet, ExcessMana, COND_None, REPNOTIFY_Always);
}

void URLAttributeSet::AdjustAttributeForMaxChange(const FGameplayAttributeData& AffectedAttribute,
                                                  const FGameplayAttributeData& MaxAttribute,
                                                  float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
{
    UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
    const float CurrentMax = MaxAttribute.GetCurrentValue();
    if (!FMath::IsNearlyEqual(CurrentMax, NewMaxValue) && ASC)
    {
        const float CurrentValue = AffectedAttribute.GetCurrentValue();
        const float NewDelta = (CurrentMax > 0.f) ? (CurrentValue * NewMaxValue / CurrentMax) - CurrentValue
                                                   : NewMaxValue;
        ASC->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}

void URLAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
    }
    else if (Attribute == GetMaxManaAttribute())
    {
        AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
    }
    else if (Attribute == GetCritChanceAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
    }
    else if (Attribute == GetHasteAttribute() || Attribute == GetMoveSpeedMultiplierAttribute())
    {
        // Never let derived multipliers drop to/below zero.
        NewValue = FMath::Max(NewValue, 0.01f);
    }
}

void URLAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        const float LocalDamage = GetIncomingDamage();
        SetIncomingDamage(0.f);
        if (LocalDamage > 0.f)
        {
            SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
        }
    }
    else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
    {
        const float LocalHealing = GetIncomingHealing();
        SetIncomingHealing(0.f);
        if (LocalHealing > 0.f)
        {
            SetHealth(FMath::Clamp(GetHealth() + LocalHealing, 0.f, GetMaxHealth()));
        }
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
    }
    else if (Data.EvaluatedData.Attribute == GetMoveSpeedMultiplierAttribute())
    {
        // Push the new movement multiplier onto the owning character.
        if (ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
        {
            if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
            {
                // 500 is the template's base MaxWalkSpeed.
                Movement->MaxWalkSpeed = 500.f * FMath::Max(GetMoveSpeedMultiplier(), 0.01f);
            }
        }
    }
}

void URLAttributeSet::OnRep_Health(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, Health, Old); }
void URLAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, MaxHealth, Old); }
void URLAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, HealthRegen, Old); }
void URLAttributeSet::OnRep_Mana(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, Mana, Old); }
void URLAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, MaxMana, Old); }
void URLAttributeSet::OnRep_ManaRegen(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, ManaRegen, Old); }
void URLAttributeSet::OnRep_Strength(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, Strength, Old); }
void URLAttributeSet::OnRep_Agility(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, Agility, Old); }
void URLAttributeSet::OnRep_Intellect(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, Intellect, Old); }
void URLAttributeSet::OnRep_CritChance(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, CritChance, Old); }
void URLAttributeSet::OnRep_Haste(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, Haste, Old); }
void URLAttributeSet::OnRep_Adaptability(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, Adaptability, Old); }
void URLAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, AttackPower, Old); }
void URLAttributeSet::OnRep_SpellPower(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, SpellPower, Old); }
void URLAttributeSet::OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, MoveSpeedMultiplier, Old); }
void URLAttributeSet::OnRep_ExcessMana(const FGameplayAttributeData& Old) { GAMEPLAYATTRIBUTE_REPNOTIFY(URLAttributeSet, ExcessMana, Old); }
