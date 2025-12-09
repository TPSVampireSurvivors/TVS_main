// Fill out your copyright notice in the Description page of Project Settings.

#include "BossTask/BossProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "TVS_main.h"

// ABossProjectile.cpp
ABossProjectile::ABossProjectile()
{
	// 투사체는 Tick이 굳이 필요 없습니다. (Movement가 알아서 함)
	PrimaryActorTick.bCanEverTick = false;

	// 1. 충돌체 설정
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f); // 기본 크기
	// 중요: 충돌 프로필을 'Projectile'로 하거나, 없으면 'BlockAllDynamic' 등 적절한 걸로 설정
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = CollisionComp;

	// 2. 메쉬(껍데기) 설정 - 충돌체 자식으로 붙임
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 껍데기는 충돌 판정 끔 (Sphere가 하니까)

	// 3. 파티클(트레일) 설정
	//ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	//ParticleComp->SetupAttachment(CollisionComp);

	// 4. 움직임 컴포넌트 설정 (핵심!)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true; // 날아가는 방향으로 머리 돌리기
	ProjectileMovement->bShouldBounce = false; // 튕기기 끔
	ProjectileMovement->ProjectileGravityScale = 0.0f; // 기본은 직사 (중력 0)

	// 충돌 이벤트 연결
	CollisionComp->OnComponentHit.AddDynamic(this, &ABossProjectile::OnHit);
}

void ABossProjectile::BeginPlay()
{
	Super::BeginPlay();

	AActor* MyOwner = GetOwner();
	if (MyOwner && CollisionComp)
	{
		// 2. "너(CollisionComp) 움직일 때, 우리 주인님(MyOwner)은 제발 무시해라" 라고 설정
		CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
	}


	if (bIsPlasma)
	{
		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (PlayerCharacter && ProjectileMovement)
		{
			// 플레이어의 RootComponent(캡슐)를 타겟으로 잡습니다.
			ProjectileMovement->HomingTargetComponent = PlayerCharacter->GetRootComponent();

			// [중요] 유도 가속도 설정: 이 값이 커야 급커브를 돌 수 있습니다.
			// 솟구치는 속도가 빠르다면 이 값도 꽤 커야(예: 5000~20000) 땅으로 잘 꽂힙니다.
			ProjectileMovement->HomingAccelerationMagnitude = 20000.0f;
		}
		// [플라즈마 모드 로직]
		if (ProjectileMovement)
		{
			// 1. 처음엔 호밍을 끕니다.
			ProjectileMovement->bIsHomingProjectile = false;

			// 2. 위쪽 방향으로 속도를 강제 설정합니다.
			// (GetActorUpVector()는 발사체의 위쪽, FVector::UpVector는 월드의 하늘 방향)
			ProjectileMovement->Velocity = GetActorUpVector() * PlasmaRiseSpeed;
		}

		// 3. 지정된 시간 뒤에 EnablePlasmaHoming 함수가 실행되도록 타이머를 겁니다.
		GetWorldTimerManager().SetTimer(
			PlasmaTimerHandle,
			this,
			&ABossProjectile::EnablePlasmaHoming,
			PlasmaRiseDuration,
			false // 반복 안 함
		);
	}

	// 안전장치: 5초 뒤에는 무조건 사라짐 (무한히 날아가는 것 방지)
	SetLifeSpan(5.0f);
}

void ABossProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 유효성 검사 (나 자신, 주인(보스), 이미 죽은 놈 무시)
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	//UE_LOG(LogBossTest, Error, TEXT("Projectile_Hit"));

	// 4. 임무 완료 - 자폭
	Destroy();
}

void ABossProjectile::EnablePlasmaHoming()
{
	// 1. 일단 유도 기능을 켜고, 최대 파워로 시작
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 30000.f; // 초기 강한 유도 (예시 값)

	// 2. 람다(Lambda)를 사용해 즉석에서 타이머 예약
	// 별도의 함수 선언 없이 여기에 로직을 다 적습니다.
	FTimerHandle TempHandle; // 멤버 변수 필요 없음. 여기서 쓰고 버림.

	GetWorld()->GetTimerManager().SetTimer(TempHandle, [this]()
		{
			// 0.1초 뒤에 실행될 내용
			if (IsValid(this) && ProjectileMovement)
			{
				// 원하는 '약한 유도' 값으로 변경
				ProjectileMovement->HomingAccelerationMagnitude = 1000.f;
			}
		}, 0.1f, false); // 0.1초 뒤 실행, 반복 없음
}