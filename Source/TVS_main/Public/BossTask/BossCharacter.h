// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "BossTask/BossAttackData.h"
#include "BossCharacter.generated.h"

UCLASS()
class TVS_MAIN_API ABossCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABossCharacter();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	const class UBossAttackData* CurrentAttackData;

	//히트박스 관리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* RightFistBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBoxComponent* LeftFistBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCapsuleComponent* BodyChargeBox; // 몸통은 캡슐이 더 자연스럽습니다

	//직접 짠 코드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* SlamSphere;


	// 오버랩 함수는 하나로 재사용 가능합니다!
	UFUNCTION()
	void OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Animation")
	UAnimMontage* AttackMontage;

	void InitAttack(const UBossAttackData* NewAttackData);

	// 블루프린트(애님 노티파이)에서 부를 수 있게 UFUNCTION 붙임
	UFUNCTION(BlueprintCallable)
	void ActivateHitbox(bool bActive);

	void ExecuteAttackEffect();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	const UBossAttackData* GetCurrentAttackData() const { return CurrentAttackData; }

	//투사체 발사 함수
	void FireProjectile(FRotator FireRotation = FRotator::ZeroRotator);

	int MaxHP;
	int CurrentHP;

	//시작용 패턴
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Config")
	class UBossAttackData* StartPattern;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHealth = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHealth;
};
