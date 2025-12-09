// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/BossAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBossAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UBossAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter == nullptr) {
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	}
	else {
		FVector Velocity = OwnerCharacter->GetVelocity();
		GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

		bIsMoving = GroundSpeed > 3.0f;
	}
}
