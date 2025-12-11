// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 
#include "JumpEnemyActor.generated.h"

UCLASS()
class SURVIVAL_API AJumpEnemyActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AJumpEnemyActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 콜리전 박스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* boxComp;

	// ★ 스켈레탈 메시 (StaticMesh → SkeletalMesh로 변경)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMeshComponent* meshComp;

	// 점프 상태 관련 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPreparingJump = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsJumping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float jumpPrepareTime = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float jumpDuration = 0.8f;

	float elapsedJumpTime = 0.0f;

	FVector jumpStart;
	FVector jumpVelocity;

	FTimerHandle PrepareTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float moveSpeed = 200.f;
	class AActor* PlayerActor;
	UFUNCTION()
	void OnEnemyOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};