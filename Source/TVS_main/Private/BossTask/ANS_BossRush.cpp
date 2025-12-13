#include "BossTask/ANS_BossRush.h"
#include "BossTask/BossAttackData.h"
#include "BossTask/BossCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void UANS_BossRush::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ABossCharacter* Boss = Cast<ABossCharacter>(MeshComp->GetOwner()))
	{
		Boss->GetCharacterMovement()->GroundFriction = 0.0f;
		Boss->GetCharacterMovement()->BrakingDecelerationWalking = 0.0f;

		if (bIsChasingMode)
		{
			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
				{
					AnimInst->Montage_SetPlayRate(Montage, 0.01f);
				}
			}
		}

		const UBossAttackData* Data = Boss->GetCurrentAttackData();

		if (Data && Data->AttackVFX)
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
				Data->AttackVFX,   
				Boss->GetMesh(),
				Data->VFXSocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget
			);

			if (PSC)
			{
				PSC->ComponentTags.Add(FName("RushBooster"));

				PSC->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));     
				PSC->SetRelativeRotation(Rot); 
				PSC->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
			}
		}
	}
}

void UANS_BossRush::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    ACharacter* Boss = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Boss) return;

	if (bIsChasingMode)
	{
		ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Boss, 0);
		if (!Player) return;

		float Dist = Boss->GetDistanceTo(Player);

		if (Dist > StopDistance)
		{
			FVector DashVelocity = Boss->GetActorForwardVector() * RushSpeed;
			DashVelocity.Z = Boss->GetCharacterMovement()->Velocity.Z;
			Boss->GetCharacterMovement()->Velocity = DashVelocity;
		}
		else
		{
			Boss->GetCharacterMovement()->StopMovementImmediately();

			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
				{
					AnimInst->Montage_SetPlayRate(Montage, 1.0f);
				}
			}
		}
	}
	else {
		FVector WorldDir = Boss->GetActorRotation().RotateVector(MoveDirection);
		WorldDir.Z = 0.0f;
		WorldDir.Normalize();

		FVector StepVelocity = WorldDir * RushSpeed;
		StepVelocity.Z = Boss->GetCharacterMovement()->Velocity.Z;

		Boss->GetCharacterMovement()->Velocity = StepVelocity;

	}
}

void UANS_BossRush::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ACharacter* Boss = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		Boss->GetCharacterMovement()->StopMovementImmediately();

		Boss->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		Boss->GetCharacterMovement()->GroundFriction = 8.0f;
		Boss->GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;


		TArray<UActorComponent*> Components;
		Boss->GetComponents(UParticleSystemComponent::StaticClass(), Components);

		for (UActorComponent* Comp : Components)
		{
			if (Comp->ComponentHasTag(FName("RushBooster")))
			{
				Comp->DestroyComponent();
			}
		}
	}
}