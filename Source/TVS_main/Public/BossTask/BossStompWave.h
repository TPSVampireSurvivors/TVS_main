// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossStompWave.generated.h"

class UParticleSystemComponent;

UCLASS()
class TVS_MAIN_API ABossStompWave : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABossStompWave();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UParticleSystemComponent* SmokeComp;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
	float MaxScaleXY = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
	float ExpansionSpeed = 5.0f;


private:
	FVector CurrentScale;

};
