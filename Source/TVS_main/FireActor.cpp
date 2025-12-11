// Fill out your copyright notice in the Description page of Project Settings.

#include "FireActor.h"
#include "FireBall.h"

// Sets default values
AFireActor::AFireActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFireActor::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(10.0f);

}

// Called every frame
void AFireActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (currentTime > delayTime)
	{
		currentTime = 0;

		AFireBall* spawnActor = GetWorld()->SpawnActor<AFireBall>(ball, GetActorLocation(), GetActorRotation());
		if (spawnActor)
		{
			spawnActor->MoveDir = GetActorRotation().Vector();
		}
	}
	else
	{
		currentTime += DeltaTime;
	}
}