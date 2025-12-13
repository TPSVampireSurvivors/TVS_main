// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/AN_AttackHit.h"
#include "BossTask/BossCharacter.h" 
#include <Kismet/GameplayStatics.h>


void UAN_AttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(MeshComp->GetWorld(), 0);
	if (!Player) return;
	UGameplayStatics::ApplyDamage(Player, 1.0f, nullptr, nullptr, nullptr);

}

