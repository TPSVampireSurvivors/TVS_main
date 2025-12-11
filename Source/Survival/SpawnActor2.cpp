// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnActor2.h"
#include "JumpEnemyActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
// Sets default values
ASpawnActor2::ASpawnActor2()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASpawnActor2::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(10.0f);
	GetWorld()->GetTimerManager().SetTimer(
		spawnTimer,
		this,
		&ASpawnActor2::SpawnEnemy,
		spawnInterval,
		true
	);
}

void ASpawnActor2::SpawnEnemy()
{
	if (enemyFactory == nullptr) return;

	FVector pos = GetActorLocation();
	FRotator rot = GetActorRotation();

	AJumpEnemyActor* enemy=GetWorld()->SpawnActor<AJumpEnemyActor>(enemyFactory, pos, rot);
	if (enemy)
	{
		enemy->SetLifeSpan(10.0f); // ★ 10초 뒤 자동 파괴
	}
}


