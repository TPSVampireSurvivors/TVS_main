// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TVS_mainCharacter.generated.h"

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
class ATVS_mainCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* capsuleComp;

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
	ATVS_mainCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ia_Dash;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	FVector2D LastMoveInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DodgeDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DodgeDuration = 0.2f;

	bool bIsDodging = false;
	FVector DodgeDirection;
	FTimerHandle DodgeTimer;
	UFUNCTION()
	void Dash();        // Dash 시작

	UFUNCTION()
	void EndDodge();    // Dash 종료
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashSpeed = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashTime = 0.2f;

	float CurrentDashTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bInvincible = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float InvincibleTime = 0.3f;
	UPROPERTY(EditAnywhere)
	class UArrowComponent* firePosition;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABullet> bulletFactory;
	FTimerHandle InvincibleTimer;
	void EndInvincible();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ia_Fire;
	void Fire();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	int32 HP = 3;
	UFUNCTION()
	void TakeEnemyDamage(int32 Damage);
	UFUNCTION()
	void OnHPChanged();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HPUIWidgetClass;
	UUserWidget* HPUI;
public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
