// Copyright Epic Games, Inc. All Rights Reserved.

#include "TVS_mainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "TVS_main.h"
#include "Components/ArrowComponent.h"
#include "Bullet.h"
#include "HPUIWidget.h"
#include "Components/Image.h"

ATVS_mainCharacter::ATVS_mainCharacter()
{
	// Set size for collision capsule
	capsuleComp = GetCapsuleComponent();
	capsuleComp->InitCapsuleSize(42.f, 96.0f);
	capsuleComp->SetCollisionProfileName(TEXT("Player"));
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	firePosition = CreateDefaultSubobject<UArrowComponent>(TEXT("FirePosition"));
	firePosition->SetupAttachment(GetMesh());
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATVS_mainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATVS_mainCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATVS_mainCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATVS_mainCharacter::Look);
		EnhancedInputComponent->BindAction(ia_Fire, ETriggerEvent::Started, this, &ATVS_mainCharacter::Fire);
		EnhancedInputComponent->BindAction(ia_Dash, ETriggerEvent::Started, this, &ATVS_mainCharacter::Dash);

	}
	else
	{
		UE_LOG(LogTVS_main, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATVS_mainCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ATVS_mainCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ATVS_mainCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ATVS_mainCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ATVS_mainCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ATVS_mainCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ATVS_mainCharacter::Dash()
{
	if (bIsDodging) return;

	bIsDodging = true;

	// 무적 시작
	bInvincible = true;
	GetWorld()->GetTimerManager().SetTimer(
		InvincibleTimer,
		this,
		&ATVS_mainCharacter::EndInvincible,
		InvincibleTime,
		false
	);

	GetCharacterMovement()->StopMovementImmediately();

	FVector InputDir = GetLastMovementInputVector();
	if (InputDir.IsNearlyZero())
	{
		InputDir = GetActorForwardVector();
	}
	InputDir.Normalize();
	DodgeDirection = InputDir;

	LaunchCharacter(DodgeDirection * DodgeDistance, true, true);

	// Dash 종료 타이머
	GetWorld()->GetTimerManager().SetTimer(
		DodgeTimer,
		this,
		&ATVS_mainCharacter::EndDodge,
		DodgeDuration,
		false
	);
}
void ATVS_mainCharacter::EndDodge()
{
	bIsDodging = false;
}
void ATVS_mainCharacter::EndInvincible()
{
	bInvincible = false;
}
void ATVS_mainCharacter::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("FIRE CALLED!"));
	if(!bulletFactory) UE_LOG(LogTemp, Warning, TEXT("BulletFactoryNull"))

	ABullet* bullet = GetWorld()->SpawnActor<ABullet>(bulletFactory,
		firePosition->GetComponentLocation(),
		firePosition->GetComponentRotation());

	if (!bullet)
		UE_LOG(LogTemp, Error, TEXT("BULLET SPAWN FAILED! (bulletFactory is NULL?)"));
}
void ATVS_mainCharacter::TakeEnemyDamage(int32 Damage)
{
	HP -= Damage;
	OnHPChanged();
	if (HP <= 0)
	{
		Destroy();
	}
}
void ATVS_mainCharacter::BeginPlay()
{
	Super::BeginPlay();

	HPUI = CreateWidget<UUserWidget>(GetWorld(), HPUIWidgetClass);
	if (HPUI)
	{
		HPUI->AddToViewport();
		// 시작할 때 하트 3개 세팅
		OnHPChanged();
	}
}
void ATVS_mainCharacter::OnHPChanged()
{
	if (HPUI)
	{
		UHPUIWidget* Widget = Cast<UHPUIWidget>(HPUI);
		if (Widget)
		{
			Widget->UpdateHearts(HP);
		}
	}
}