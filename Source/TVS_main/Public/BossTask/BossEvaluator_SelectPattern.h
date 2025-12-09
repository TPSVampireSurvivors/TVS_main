// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "BossTask/BossAttackData.h" // 데이터 에셋 헤더 경로 확인 필요
#include "BossEvaluator_SelectPattern.generated.h"

/**
 * [역할] 매 틱마다 상황을 분석하여 [다음 패턴]과 [카테고리]를 결정하는 두뇌
 */
UCLASS(meta = (DisplayName = "Boss Attack Selector"))
class TVS_MAIN_API UBossEvaluator_SelectPattern : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

public:
	// [Output] 선택된 구체적인 공격 데이터 (Task가 몽타주 재생할 때 가져다 씀)
	// State Tree 에디터의 'Output Bindings' 탭에서 트리 파라미터와 연결하세요.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
	UBossAttackData* OutSelectedPatternData;

	// [Output] 행동 카테고리 (State Tree가 어떤 가지(State)로 전이할지 결정할 때 씀)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
	EBossPatternCategory OutCategory;

	// 매 프레임 호출되는 함수
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
};