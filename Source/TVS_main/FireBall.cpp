// Fill out your copyright notice in the Description page of Project Settings.

#include "FireBall.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SurvivalCharacter.h"
// Sets default values
AFireBall::AFireBall()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComp);
	CollisionComp->InitSphereRadius(15.f);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionProfileName(TEXT("Enemy"));
}

// Called when the game starts or when spawned
void AFireBall::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);
	MeshComp->OnComponentBeginOverlap.AddDynamic(this, &AFireBall::OnEnemyOverlap);
}

// Called every frame
void AFireBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewPos = GetActorLocation() + MoveDir * Speed * DeltaTime;
	SetActorLocation(NewPos);
}
void AFireBall::OnEnemyOverlap(UPrimitiveComponent* OverlappedComponent, AActor*
	OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const
	FHitResult& SweepResult)
{
	ASurvivalCharacter* player = Cast<ASurvivalCharacter>(OtherActor);

	// 만일, 캐스팅이 성공했다면 ….
	if (player != nullptr)
	{
		player->TakeEnemyDamage(1);
	}
	Destroy();
}