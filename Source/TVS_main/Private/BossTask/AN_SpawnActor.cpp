// Fill out your copyright notice in the Description page of Project Settings.

#include "BossTask/AN_SpawnActor.h"
#include "Engine/World.h"

void UAN_SpawnActor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetWorld() || !ActorClassToSpawn)
	{
		return;
	}

	FVector SpawnLocation = MeshComp->GetComponentLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;



	SpawnLocation += LocationOffset;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	MeshComp->GetWorld()->SpawnActor<AActor>(ActorClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	
}
