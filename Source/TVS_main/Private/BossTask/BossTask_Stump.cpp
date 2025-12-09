// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/BossTask_Stump.h"
#include "BossTask/BossAttackData.h"
#include "BossTask/BossCharacter.h"
#include "StateTreeExecutionContext.h"
#include "TVS_main.h"


UBossTask_Stump::UBossTask_Stump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	bShouldCallTick = true; 
}

EStateTreeRunStatus UBossTask_Stump::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) {
	Super::EnterState(Context, Transition);

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	BossCharacter = Cast<ABossCharacter>(OwnerActor);

	if (!BossCharacter) return EStateTreeRunStatus::Failed;

	BossCharacter->InitAttack(AttackData);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UBossTask_Stump::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);



	return EStateTreeRunStatus::Running;
}