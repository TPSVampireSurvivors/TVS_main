// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/BossTask_RapidFire.h"
#include "BossTask/BossAttackData.h"
#include "BossTask/BossCharacter.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTreeExecutionContext.h"
#include "TVS_main.h"
//asd
UBossTask_RapidFire::UBossTask_RapidFire(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCallTick = true; 
}

EStateTreeRunStatus UBossTask_RapidFire::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) {
	//UE_LOG(LogBossTest, Error, TEXT("EnterState Fire"));

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	ABossCharacter* BossCharacter = Cast<ABossCharacter>(OwnerActor);
	FireTime = 0.0f;

	if (!BossCharacter || !AttackData)
	{
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(BossCharacter->GetController());
	if (AIController)
	{
		ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
		if (Player)
		{
			// 1. 플레이어 방향 및 우측 벡터 계산
			FVector BossLoc = BossCharacter->GetActorLocation();
			FVector BossRight = BossCharacter->GetActorRightVector();

			// 2. 몽타주 이름에 따라 이동 방향 결정
			float MoveDir = 0.0f;
			FString MontageName = AttackData->Montage->GetName();

			if (MontageName.Contains(TEXT("RapidPursuitL")))
			{
				MoveDir = -1.0f; // 왼쪽 (우측 벡터의 반대)
			}
			else if (MontageName.Contains(TEXT("RapidPursuitR")))
			{
				MoveDir = 1.0f; // 오른쪽
			}

			// 3. 이동 목표 설정 (예: 600유닛 이동, 필요시 거리 조절)
			if (MoveDir != 0.0f)
			{
				FVector MoveDest = BossLoc + (BossRight * MoveDir * AttackData->MoveSpeed);
				BossCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
				AIController->MoveToLocation(MoveDest);

			}
		}
	}

	BossCharacter->InitAttack(AttackData);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UBossTask_RapidFire::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) {
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	ABossCharacter* BossCharacter = Cast<ABossCharacter>(OwnerActor);
	
	if (!BossCharacter || !AttackData) return EStateTreeRunStatus::Failed;
	if (AttackData->bEnableRotation){
		TracePlayer(BossCharacter, DeltaTime);
	}

	FireBullet(BossCharacter, DeltaTime);

	UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && !AnimInstance->Montage_IsPlaying(AttackData->Montage)) {
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}


void UBossTask_RapidFire::TracePlayer(ABossCharacter* BossCharacter, float DeltaTime) {
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (Player) {
		FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(BossCharacter->GetActorLocation(), Player->GetActorLocation());
		TargetRot = FRotator(0.0f, TargetRot.Yaw, 0.0f);
		FRotator NewRot = FMath::RInterpTo(BossCharacter->GetActorRotation(), TargetRot, DeltaTime, AttackData->RotationSpeed);
		BossCharacter->SetActorRotation(NewRot);
	}
}

void UBossTask_RapidFire::FireBullet(ABossCharacter* BossCharacter, float DeltaTime) {
	//UE_LOG(LogBossTest, Error, TEXT("Enter FireBullet"));
	FireTime += DeltaTime;

	if (FireTime >= AttackData->FireCoolTime) {
		//UE_LOG(LogBossTest, Error, TEXT("Fire!"));
		BossCharacter->FireProjectile();
		FireTime -= AttackData->FireCoolTime;
	}
}

