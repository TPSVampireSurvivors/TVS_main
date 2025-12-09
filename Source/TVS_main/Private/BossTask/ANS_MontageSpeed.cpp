// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/ANS_MontageSpeed.h"
#include "BossTask/BossCharacter.h"
#include "Animation/AnimMontage.h"


void UANS_MontageSpeed::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) {
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp) {
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance()) {
			UAnimMontage* Montage = Cast<UAnimMontage>(Animation);
			Anim->Montage_SetPlayRate(Montage, PlayRate);
		}
	}
}

void UANS_MontageSpeed::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) {
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp) {
		ABossCharacter* Boss = Cast<ABossCharacter>(MeshComp->GetOwner());
		if (Boss) {
			if (UAnimInstance* Anim = MeshComp->GetAnimInstance()) {
				UAnimMontage* Montage = Cast<UAnimMontage>(Animation);
				Anim->Montage_SetPlayRate(Montage, Boss->GetCurrentAttackData()->PlayRate);
			}
		}
	}
}