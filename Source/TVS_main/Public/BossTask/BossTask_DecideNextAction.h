// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "BossTask/BossAttackData.h"
#include "BossTask_DecideNextAction.generated.h"

/**
 * [역할] 실행될 때 '딱 한 번' 다음 패턴을 계산하고 종료되는 태스크
 */

UCLASS(meta = (DisplayName = "Decide Next Action"))
class TVS_MAIN_API UBossTask_DecideNextAction : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	// [Output] 결정된 데이터를 State Tree 변수에 내보냅니다.
	UPROPERTY(EditAnywhere, Category = "Output")
	UBossAttackData* OutAttackData;

	UPROPERTY(EditAnywhere, Category = "Output")
	EBossPatternCategory OutCategory;

	// 실행될 때 1회 호출
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
};