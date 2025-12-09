// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/SpawnActor.h"
#include "Engine/World.h"

void UAnimNotify_SpawnActor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetWorld() || !ActorClassToSpawn)
	{
		return;
	}

	FVector SpawnLocation = MeshComp->GetComponentLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator; // 회전은 보통 0 (바닥에 평행)



	// 3. 오프셋 적용
	SpawnLocation += LocationOffset;

	// 4. 액터 소환 (항상 스폰되도록 CollisionHandlingOverride 설정 추천)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	MeshComp->GetWorld()->SpawnActor<AActor>(ActorClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	
}
