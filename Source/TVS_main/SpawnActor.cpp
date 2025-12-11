// EnemySpawner.cpp
#include "SpawnActor.h"
#include "EnemyActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ASpawnActor::ASpawnActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASpawnActor::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(10.0f);
	// 1초마다 SpawnEnemy 실행
	GetWorld()->GetTimerManager().SetTimer(
		spawnTimer,
		this,
		&ASpawnActor::SpawnEnemy,
		spawnInterval,
		true
	);
}

void ASpawnActor::SpawnEnemy()
{
	if (enemyFactory == nullptr) return;

	FVector pos = GetActorLocation();
	FRotator rot = GetActorRotation();

	AEnemyActor* enemy = GetWorld()->SpawnActor<AEnemyActor>(enemyFactory, pos, rot);
	if (enemy)
	{
		enemy->SetLifeSpan(10.0f); // ★ 10초 뒤 자동 파괴
	}
}
