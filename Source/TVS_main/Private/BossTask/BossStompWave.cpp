// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/BossStompWave.h"
#include "Particles/ParticleSystemComponent.h"

ABossStompWave::ABossStompWave()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    SmokeComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SmokeComp"));
    SmokeComp->SetupAttachment(RootComponent);

    SmokeComp->bAutoActivate = true;
}

void ABossStompWave::BeginPlay()
{
    Super::BeginPlay();

    CurrentScale = FVector(0.1f, 0.1f, 1.0f);
    SmokeComp->SetWorldScale3D(CurrentScale);


    SetLifeSpan(1.0f);
}

void ABossStompWave::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentScale.X < MaxScaleXY)
    {
        float NewXY = FMath::FInterpTo(CurrentScale.X, MaxScaleXY, DeltaTime, ExpansionSpeed);

        CurrentScale.X = NewXY;
        CurrentScale.Y = NewXY;

        SmokeComp->SetWorldScale3D(CurrentScale);
    }
}