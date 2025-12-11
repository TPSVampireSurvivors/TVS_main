// EnemySpawner.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnActor.generated.h"

UCLASS()
class SURVIVAL_API ASpawnActor : public AActor
{
	GENERATED_BODY()

public:
	ASpawnActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AEnemyActor> enemyFactory;

	UPROPERTY(EditAnywhere)
	float spawnInterval = 1.0f;   // 1√  ∞£∞›

	FTimerHandle spawnTimer;

	void SpawnEnemy();
};
