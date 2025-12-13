// Fill out your copyright notice in the Description page of Project Settings.

#include "BossTask/BossTask_MeleeAttack.h"
#include "BossTask/BossAttackData.h"
#include "BossTask/BossCharacter.h"
#include "TVS_main.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "StateTreeExecutionContext.h"

UBossTask_MeleeAttack::UBossTask_MeleeAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCallTick = true; // 몽타주 끝나는 거 감시해야 함
}

EStateTreeRunStatus UBossTask_MeleeAttack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	//UE_LOG(LogBossTest, Error, TEXT("Melee Enter"));
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());

	// 님 프로젝트의 실제 보스 클래스로 캐스팅
	ABossCharacter* BossCharacter = Cast<ABossCharacter>(OwnerActor);

	// 방어 코드: 보스가 없거나, 데이터 에셋을 안 끼워놨다면 실패!
	if (!BossCharacter || !AttackData)
	{
		//UE_LOG(LogBossTest, Error, TEXT("None AttackData"));
		return EStateTreeRunStatus::Failed;
	}

	AAIController* AIController = Cast<AAIController>(BossCharacter->GetController());
	if (AIController)
	{
		AIController->StopMovement();
	}

	// ✅ [핵심 변경] 보스야, 이 데이터대로 공격해라!
	// (InitAttack 함수 안에서 몽타주 재생까지 다 처리합니다)
	BossCharacter->InitAttack(AttackData);

	bIsMontagePlaying = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UBossTask_MeleeAttack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	ABossCharacter* BossCharacter = Cast<ABossCharacter>(OwnerActor);

	if (!BossCharacter || !AttackData) return EStateTreeRunStatus::Failed;

	// 2. 몽타주가 끝났는지 확인
	UAnimInstance* AnimInstance = BossCharacter->GetMesh()->GetAnimInstance();

	//  AttackData 안에 들어있는 몽타주를 체크합니다.
	if (AnimInstance && !AnimInstance->Montage_IsPlaying(AttackData->Montage))
	{
		return EStateTreeRunStatus::Succeeded; // 끝남!
	}

	// 3. 회전 로직 (이건 그대로 유지)
	if (AttackData->bEnableRotation)
	{
		ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
		if (Player)
		{
			FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(BossCharacter->GetActorLocation(), Player->GetActorLocation());
			TargetRot = FRotator(0.0f, TargetRot.Yaw, 0.0f);
			FRotator NewRot = FMath::RInterpTo(BossCharacter->GetActorRotation(), TargetRot, DeltaTime, AttackData->RotationSpeed);
			BossCharacter->SetActorRotation(NewRot);
		}
	}

	return EStateTreeRunStatus::Running;
}