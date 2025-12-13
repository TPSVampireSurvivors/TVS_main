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
		CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
	}

	if (bIsPlasma)
	{
		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (PlayerCharacter && ProjectileMovement)
		{
			ProjectileMovement->HomingTargetComponent = PlayerCharacter->GetRootComponent();

			ProjectileMovement->HomingAccelerationMagnitude = 20000.0f;
		}
		if (ProjectileMovement)
		{
			ProjectileMovement->bIsHomingProjectile = false;

			ProjectileMovement->Velocity = GetActorUpVector() * PlasmaRiseSpeed;
		}

		GetWorldTimerManager().SetTimer(
			PlasmaTimerHandle,
			this,
			&ABossProjectile::EnablePlasmaHoming,
			PlasmaRiseDuration,
			false 
		);
	}

	SetLifeSpan(5.0f);
}

void ABossProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 유효성 검사 (나 자신, 주인(보스), 이미 죽은 놈 무시)
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	if (OtherActor) {
		ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);
		if (HitCharacter && HitCharacter->IsPlayerControlled())
		{
			UGameplayStatics::ApplyDamage(
				HitCharacter,              
				1.0f,                      
				GetInstigatorController(),
				this,                        
				UDamageType::StaticClass()   
			);
		}
	}

	Destroy();
}

void ABossProjectile::EnablePlasmaHoming()
{
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingAccelerationMagnitude = 30000.f; 

	FTimerHandle TempHandle; 

	GetWorld()->GetTimerManager().SetTimer(TempHandle, [this]()
		{
			if (IsValid(this) && ProjectileMovement)
			{
				ProjectileMovement->HomingAccelerationMagnitude = 1000.f;
			}
		}, 0.1f, false); 
}