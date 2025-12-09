// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "BossTask_Takedown.generated.h"

class UBossAttackData;
/**
 * 
 */
UCLASS()
class TVS_MAIN_API UBossTask_Takedown : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()


public:
	UBossTask_Takedown(const FObjectInitializer& ObjectInitializer);
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UBossAttackData> AttackData;

	// 3. 연출 (VFX / SFX)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	UParticleSystem* TakedownVFX; // 공격 시 터지는 이펙트

private:
	// 내부 로직 진행 상태
	enum class ETakedownState
	{
		Preparing,  // Start 몽타주
		Jumping,    // Loop 몽타주 (공중)
		Recovering, // Land 몽타주 (착지)
		Finished    // 완료
	};

	void StartAscend();
	void StartHover();
	void PerformImpactLogic();
	void StartSmash();
	void TakedownEffect();

	// --- 설정 변수 (에디터에서 할당) ---
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* Montage_Start;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* Montage_Loop;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* Montage_Land;


	// --- 델리게이트 함수 ---
	UFUNCTION()
	void OnStartMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnCharacterLanded(const FHitResult& Hit);

	UFUNCTION()
	void OnLandMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	ETakedownState InternalState;
	FDelegateHandle LandedDelegateHandle;

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> BossCharacter;

	FTimerHandle TimerHandle_Hover;
	FTimerHandle TimerHandle_Smash;

	//// 내부 동작 함수들
	//void StartJump();
	//void StartHover();
	//void StartSlam();
	//void OnLanded();
};
