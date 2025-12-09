// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_BossRush.generated.h"

/**
 * 
 */
UCLASS()
class TVS_MAIN_API UANS_BossRush : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	// 돌진 속도 (에디터에서 조절 가능)
	UPROPERTY(EditAnywhere, Category = "Rush")
	float RushSpeed = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Rush")
	float StopDistance = 1000.0f;

	// ✅ [추가] 이동 방향 (X=앞뒤, Y=좌우)
	// 예: (1,0,0)=앞, (-1,0,0)=뒤, (0,1,0)=오른쪽, (0,-1,0)=왼쪽
	UPROPERTY(EditAnywhere, Category = "Rush")
	FVector MoveDirection = FVector(1.0f, 0.0f, 0.0f);

	//직접 쓴 코드
	UPROPERTY(EditAnywhere, Category = "Rush")
	bool bIsChasingMode = true;

	UPROPERTY(EditAnywhere, Category = "VFX")
	FRotator Rot = FRotator(0.0f, 0.0f, 0.0f);

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
