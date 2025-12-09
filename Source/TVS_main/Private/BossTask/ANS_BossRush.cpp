#include "BossTask/ANS_BossRush.h"
#include "BossTask/BossAttackData.h"
#include "BossTask/BossCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

void UANS_BossRush::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ABossCharacter* Boss = Cast<ABossCharacter>(MeshComp->GetOwner()))
	{
		// 1. 마찰력 제거 (미끄러짐)
		Boss->GetCharacterMovement()->GroundFriction = 0.0f;
		Boss->GetCharacterMovement()->BrakingDecelerationWalking = 0.0f;

		// 2. 시간 정지 (추적 모드일 때만)
		if (bIsChasingMode)
		{
			// if문 중첩으로 안전하게 처리
			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
				{
					AnimInst->Montage_SetPlayRate(Montage, 0.01f);
				}
			}
		}

		// 3. 이펙트 생성 (단순화됨: 무조건 AttackVFX 사용)
		const UBossAttackData* Data = Boss->GetCurrentAttackData();

		// 데이터 에셋이 있고 + AttackVFX(부스터용 불꽃)가 설정되어 있다면?
		if (Data && Data->AttackVFX)
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
				Data->AttackVFX,      // DA_Dash에 넣어둔 Flamethrower
				Boss->GetMesh(),
				Data->VFXSocketName,  // 소켓 이름 (예: Spine_02)
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget
			);

			if (PSC)
			{
				// ✅ [중요] 끄기 위한 태그 (NotifyEnd랑 철자 꼭 맞추세요!)
				PSC->ComponentTags.Add(FName("RushBooster"));

				// ✅ [부스터 전용 보정]
				// AttackVFX를 쓰지만, 이 기술(Rush)에서는 작고 뒤집혀서 나와야 함
				PSC->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));       // 작게
				PSC->SetRelativeRotation(Rot); // 뒤로 뿜기
				PSC->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
			}
		}
	}
}

void UANS_BossRush::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    ACharacter* Boss = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Boss) return;

	if (bIsChasingMode)
	{
		ACharacter* Player = UGameplayStatics::GetPlayerCharacter(Boss, 0);
		if (!Player) return;

		// 1. 플레이어와의 거리 계산 (3D 거리)
		float Dist = Boss->GetDistanceTo(Player);

		// (옵션: 높이차 무시하고 수평 거리만 재려면 아래 코드 사용)
		// float Dist = FVector::Dist2D(Boss->GetActorLocation(), Player->GetActorLocation());


		// 2. 아직 목표 거리보다 멀리 있다면? -> 계속 전진!
		if (Dist > StopDistance)
		{
			FVector DashVelocity = Boss->GetActorForwardVector() * RushSpeed;
			DashVelocity.Z = Boss->GetCharacterMovement()->Velocity.Z;
			Boss->GetCharacterMovement()->Velocity = DashVelocity;
		}
		// 3. 목표 거리 안으로 들어왔다면? -> 브레이크!
		else
		{
			// A. 물리적 정지
			Boss->GetCharacterMovement()->StopMovementImmediately();

			// ✅ [핵심 2] 도착했으니 시간을 다시 흐르게 합니다! (착지 동작 재생)
			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
				{
					// 정상 속도로 복구 -> 남은 막대기 구간이 순식간에 지나가고 NotifyEnd가 호출됨
					AnimInst->Montage_SetPlayRate(Montage, 1.0f);
				}
			}
		}
	}
	else {
		//직접 쓴 코드
		FVector WorldDir = Boss->GetActorRotation().RotateVector(MoveDirection);
		WorldDir.Z = 0.0f;
		WorldDir.Normalize();

		FVector StepVelocity = WorldDir * RushSpeed;
		StepVelocity.Z = Boss->GetCharacterMovement()->Velocity.Z;

		Boss->GetCharacterMovement()->Velocity = StepVelocity;

	}
}

void UANS_BossRush::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ACharacter* Boss = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		// 5. 끝나면 브레이크!
		Boss->GetCharacterMovement()->StopMovementImmediately();

		// 6. 원래 속성 복구 (수치는 님 프로젝트 기본값으로 맞추세요)
		Boss->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		Boss->GetCharacterMovement()->GroundFriction = 8.0f; // 기본 마찰력
		Boss->GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f; // 기본 제동력


		TArray<UActorComponent*> Components;
		// 보스가 가진 모든 파티클 컴포넌트를 가져옵니다.
		Boss->GetComponents(UParticleSystemComponent::StaticClass(), Components);

		for (UActorComponent* Comp : Components)
		{
			// 아까 달아둔 "RushBooster" 태그가 있는지 확인
			if (Comp->ComponentHasTag(FName("RushBooster")))
			{
				// 찾았다! 즉시 제거.
				Comp->DestroyComponent();
			}
		}
	}
}