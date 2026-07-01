// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"   // up top with other includes
// change the class declaration to also inherit the interface:
// class ARELIQUARYCharacter : public ACharacter, public IAbilitySystemInterface

#include "RELIQUARYCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ARELIQUARYCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	ARELIQUARYCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	virtual void PossessedBy(AController* NewController) override;

	/**
	 *  Interact with the nearest thing in front of the character that implements
	 *  IRLInteractable — altars, banking crates, the forge. Bind this to an input
	 *  action in the character/controller blueprint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool DoInteract();

	/** Find (without activating) the interactable currently in focus, if any. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* FindFocusedInteractable() const;

protected:
	UPROPERTY() TObjectPtr<class UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY() TObjectPtr<class URLAttributeSet> Attributes;
	void InitGAS();

	/** How far ahead to look for interactables. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionRange = 250.f;

	/** Base-camp map opened when the character dies on a run. */
	UPROPERTY(EditAnywhere, Category = "RELIQUARY")
	FName BaseCampLevelName = TEXT("L_Lobby");

	/** Called when Health changes; drives death handling. */
	void OnHealthAttributeChanged(const struct FOnAttributeChangeData& Data);

	/** Forfeit the run, strip run power, and send the player home to base camp. */
	void HandleRunDeath();

	/** Content hook: play the death reaction before travel to base camp. */
	UFUNCTION(BlueprintImplementableEvent, Category = "RELIQUARY")
	void OnRunDeath();

	bool bIsDead = false;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:
	virtual void Tick(float DeltaSeconds) override;

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

