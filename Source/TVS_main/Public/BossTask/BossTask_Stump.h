// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "BossTask_Stump.generated.h"

class UBossAttackData;
class ABossCharacter;
/**
 * 
 */
UCLASS()
class TVS_MAIN_API UBossTask_Stump : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
public:
	UBossTask_Stump(const FObjectInitializer& ObjectInitializer);

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	// ✅ 에디터에서 공격 몽타주를 지정할 수 있게 노출
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	class UBossAttackData* AttackData;

private:

	UPROPERTY(Transient)
	ABossCharacter* BossCharacter;
	
};
