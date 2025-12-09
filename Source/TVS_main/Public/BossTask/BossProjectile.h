// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossProjectile.generated.h"

UCLASS()
class TVS_MAIN_API ABossProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABossProjectile();

private:
	virtual void BeginPlay() override;

public:
	// 날아가는 움직임을 담당하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	// 충돌체
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;



	//플라즈마 패턴 변수
	// 이 값이 True면 발사 시 공중으로 솟구쳤다가 타겟을 향해 날아갑니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Plasma")
	bool bIsPlasma = false;

	// 공중으로 솟구치는 시간(초)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Plasma", meta = (EditCondition = "bIsPlasma"))
	float PlasmaRiseDuration = 0.5f;

	// 솟구칠 때의 상승 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Plasma", meta = (EditCondition = "bIsPlasma"))
	float PlasmaRiseSpeed = 1000.0f;



private:
	// 무언가에 부딪혔을 때 호출될 함수
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 상승 단계가 끝나고 호밍을 시작하기 위해 호출되는 함수
	void EnablePlasmaHoming();

	// 타이머 핸들 (상승 시간 체크용)
	FTimerHandle PlasmaTimerHandle;
};
