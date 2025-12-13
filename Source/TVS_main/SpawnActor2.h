// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnActor2.generated.h"

UCLASS()
class TVS_MAIN_API ASpawnActor2 : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnActor2();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AJumpEnemyActor> enemyFactory;

	UPROPERTY(EditAnywhere)
	float spawnInterval = 1.0f;   // 1√  ∞£∞›

	FTimerHandle spawnTimer;

	void SpawnEnemy();
};