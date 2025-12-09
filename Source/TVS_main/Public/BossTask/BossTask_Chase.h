// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "BossTask_Chase.generated.h"

/**
 * 
 */
UCLASS()
class TVS_MAIN_API UBossTask_Chase : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	class AAIController* OwnerController;

public:
	UBossTask_Chase(const FObjectInitializer& ObjectInitializer);

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition);
};
