#include "BossTask/ANS_AttackHitCheck.h"
#include "BossTask/BossCharacter.h" // 보스 헤더 필요
#include "TVS_main.h"


void UANS_AttackHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		ABossCharacter* Boss = Cast<ABossCharacter>(MeshComp->GetOwner());
		if (Boss)
		{
			//UE_LOG(LogBossTest, Error, TEXT("NotifyBegin"));
			// 시작될 때 켬 (Index에 맞는 부위만)
			Boss->ActivateHitbox(true);
		}
	}
}

void UANS_AttackHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		ABossCharacter* Boss = Cast<ABossCharacter>(MeshComp->GetOwner());
		if (Boss)
		{
			//UE_LOG(LogBossTest, Error, TEXT("NotifyEnd"));
			// 끝날 때 끔 (Index에 맞는 부위만)
			Boss->ActivateHitbox(false);
		}
	}
}