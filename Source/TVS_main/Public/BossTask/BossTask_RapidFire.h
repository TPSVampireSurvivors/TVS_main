// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "BossTask/BossAttackData.h"
#include "BossTask_RapidFire.generated.h"

/**
 * 
 */
UCLASS()
class TVS_MAIN_API UBossTask_RapidFire : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	

public:
	UBossTask_RapidFire(const FObjectInitializer& ObjectInitializer);

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;


private:
	//패턴 데이터 에셋
	UPROPERTY(EditAnywhere, Category = "Parameter")
	class UBossAttackData* AttackData;

	//몽타주 끝?
	bool bIsMontagePlaying = false;


	float FireTime = 0.0f;


	//내부 함수
	//플레이어 바라보기, 총 쏘기
	void TracePlayer(class ABossCharacter* BossCharacter, float DeltaTime);
	void FireBullet(class ABossCharacter* BossCharacter, float DeltaTime);

};
