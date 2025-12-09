// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SpawnActor.generated.h"

/**
 * 
 */
UCLASS()
class TVS_MAIN_API UAnimNotify_SpawnActor : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSubclassOf<AActor> ActorClassToSpawn;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector LocationOffset;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};