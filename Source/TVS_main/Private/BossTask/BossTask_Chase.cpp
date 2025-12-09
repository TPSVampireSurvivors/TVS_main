// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/BossTask_Chase.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBossAI, Display, All);

UBossTask_Chase::UBossTask_Chase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer){
	bShouldCallTick = true; // Tick을 쓰려면 생성자에서 켜줘야 합니다.
}

EStateTreeRunStatus UBossTask_Chase::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	// 1. 이 Task를 실행한 주체(Actor) 가져오기
	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());

	if (!OwnerActor)
	{
		//UE_LOG(LogBossAI, Error, TEXT("EnterState Failed: Owner is missing."));
		return EStateTreeRunStatus::Failed;
	}
	// 2. Pawn -> AIController 가져오기
	APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	if (OwnerPawn)
	{
		OwnerController = Cast<AAIController>(OwnerPawn->GetController());
	}

	if (!OwnerController)
	{
		//UE_LOG(LogBossAI, Error, TEXT("EnterState Failed: AIController missing. Check 'Auto Possess AI'."));
		return EStateTreeRunStatus::Failed;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player) return EStateTreeRunStatus::Failed;

	// ✅ 이동 명령은 진입할 때 '한 번만' 내립니다.
	// bUsePathfinding = true, bCanStrafe = true 등으로 설정 가능
	EPathFollowingRequestResult::Type Result = OwnerController->MoveToActor(Player, 10.0f);

	if (Result == EPathFollowingRequestResult::Failed)
	{
		return EStateTreeRunStatus::Failed; // 이동 불가능한 위치면 실패 처리
	}


	return EStateTreeRunStatus::Running; // "나 지금 실행 중이야!"라고 알림
}

EStateTreeRunStatus UBossTask_Chase::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	if (!OwnerController) return EStateTreeRunStatus::Failed;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player) return EStateTreeRunStatus::Failed;

	// 1. 거리 체크 (성공 조건)
	float Dist = OwnerController->GetPawn()->GetDistanceTo(Player);
	if (Dist <= 150.0f)
	{
		// 도착했으므로 이동 멈춤
		OwnerController->StopMovement();
		return EStateTreeRunStatus::Succeeded; // -> 공격 State로 전환
	}

	if (OwnerController->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		// 도착도 안 했는데 멈췄다? = 길을 잃었거나 막힘
		//(LogBossAI, Warning, TEXT("Chase Stuck: Movement stopped but target not reached."));
		return EStateTreeRunStatus::Failed;
	}

	if (!OwnerController->GetPathFollowingComponent()->HasValidPath())
	{
		// 경로가 없는데 도착도 안 했다? -> 실패!
		return EStateTreeRunStatus::Failed;
	}


	return EStateTreeRunStatus::Running; // 아직 멀었으니 계속 실행
}

void UBossTask_Chase::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	// 안전장치: 상태가 바뀌면 무조건 멈춘다.
	if (OwnerController)
	{
		OwnerController->StopMovement();
	}

	// 로그 확인용
	//UE_LOG(LogBossAI, Display, TEXT("Chase State Exited. Movement Stopped."));
}