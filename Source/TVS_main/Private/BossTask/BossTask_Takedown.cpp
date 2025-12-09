// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/BossTask_Takedown.h"
#include "BossTask/BossCharacter.h"
#include "BossTask/BossAttackData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "StateTreeExecutionContext.h"
#include "DrawDebugHelpers.h"
#include "TVS_main.h"
#include <Kismet/KismetMathLibrary.h>
// ... (ExecuteTask, TickTask 등은 동일하므로 생략, 변수 접근만 바뀜) ...

UBossTask_Takedown::UBossTask_Takedown(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bShouldCallTick = true;
}

EStateTreeRunStatus UBossTask_Takedown::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    // [중요] 부모 클래스 로직 실행 (블루프린트 노드 호환성 위해 필수)
    Super::EnterState(Context, Transition);

    //UE_LOG(LogBossTest, Error, TEXT("Takedown Enter"));

    // Context Owner 가져오기
    AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
    BossCharacter = Cast<ACharacter>(OwnerActor);

    if (!BossCharacter || !Montage_Start)
    {
        //UE_LOG(LogBossTest, Error, TEXT("Failed"));
        return EStateTreeRunStatus::Failed;
    }

    UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance();
    if (!AnimInst) return EStateTreeRunStatus::Failed;

    // [1단계] Start 몽타주 재생
    InternalState = ETakedownState::Preparing;
    //UE_LOG(LogBossTest, Error, TEXT("1 Enter"));

    float Duration = AnimInst->Montage_Play(Montage_Start);
    if (Duration > 0.f)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UBossTask_Takedown::OnStartMontageEnded);
        AnimInst->Montage_SetEndDelegate(EndDelegate, Montage_Start);
    }
    else
    {
        // 몽타주 재생 실패 시 바로 다음 단계로
        OnStartMontageEnded(Montage_Start, false);
    }

    return EStateTreeRunStatus::Running;
}

void UBossTask_Takedown::OnStartMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    //UE_LOG(LogBossTest, Error, TEXT("2 Enter"));
    if (bInterrupted || InternalState != ETakedownState::Preparing) return;

    InternalState = ETakedownState::Jumping;

    // 1. 점프 계산 및 실행
    StartAscend();

    // 2. Loop 몽타주 재생
    if (UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance())
    {
        if (Montage_Loop) AnimInst->Montage_Play(Montage_Loop);
    }
}

void UBossTask_Takedown::StartAscend()
{
    if (!BossCharacter) return;

    // 중력 무시하고 강제로 밀어올리기 위해 잠시 중력 끔 (선택 사항이나, 정확한 시간 제어를 위해 추천)
    // 혹은 중력을 켠 상태로 하려면 V = g * t 공식을 써야 함.
    // 여기서는 "연출"을 위해 LaunchCharacter로 강하게 띄웁니다.

    // 안전장치: 데이터가 없으면 기본값 사용 (혹은 리턴)
    float TargetHeight = (AttackData) ? AttackData->JumpHeight : 5000.0f;
    float ReachTime = (AttackData) ? AttackData->HoverDuration : 3.0f;

    // 단순 무식하게 속도 계산: 거리 / 시간
    // (중력 영향을 고려하면 더 복잡하지만, BossCharacter는 보통 강제 힘으로 띄우는 게 속 편함)
    float UpVelocity = TargetHeight / ReachTime;
    BossCharacter->GetCharacterMovement()->GravityScale = 0.0f;

    FVector LaunchVel = FVector(0, 0, UpVelocity);
    BossCharacter->LaunchCharacter(LaunchVel, true, true);

    //애니메이션 시작
    if (AttackData && AttackData->AttackVFX)
    {
        // 소켓 이름이 없으면 등(spine_02)으로 설정
        FName SocketName = AttackData->VFXSocketName.IsNone() ? FName("spine_02") : AttackData->VFXSocketName;

        UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
            AttackData->AttackVFX,
            BossCharacter->GetMesh(),
            SocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget
        );

        if (PSC)
        {
            // 나중에 끄기 위해 태그 부착 (이름은 TakedownBooster로 설정)
            PSC->ComponentTags.Add(FName("TakedownBooster"));

            // 등 뒤로 뿜어져 나오게 회전 및 위치 조정
            FRotator BoosterRot = FRotator(180.0f, 0.0f, 0.0f);
            FVector BoosterOffset = FVector(0.0f, 0.0f, 150.0f);

            PSC->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
            PSC->SetRelativeRotation(BoosterRot);
            PSC->SetRelativeLocation(BoosterOffset);
        }
    }

    // 2. 루프 몽타주 재생
    if (UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance())
    {
        if (Montage_Loop) AnimInst->Montage_Play(Montage_Loop);
    }

    // 3. 착지 델리게이트는 아직 바인딩 하지 않음 (올라가는 중이니까)

    // 4. 상승 시간(3.0초) 후에 Hover 시작하도록 타이머 설정
    BossCharacter->GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_Hover,
        this,
        &UBossTask_Takedown::StartHover,
        ReachTime,
        false
    );
}

// --------------------------------------------------------
// [단계 2] 공중 부양 (Hover Time: 3.0초)
// --------------------------------------------------------
void UBossTask_Takedown::StartHover()
{
    if (!BossCharacter) return;

    BossCharacter->GetCharacterMovement()->StopMovementImmediately();
    BossCharacter->GetCharacterMovement()->GravityScale = 0.0f;

    // 플레이어 바라보기 로직 ...

    // 체공 시간 데이터 사용
    float HoverDuration = (AttackData) ? AttackData->HoverDuration : 3.0f;

    BossCharacter->GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_Smash,
        this,
        &UBossTask_Takedown::StartSmash,
        HoverDuration,
        false
    );
}

// [3단계] 내리찍기 (StartSmash)
void UBossTask_Takedown::StartSmash()
{
    if (!BossCharacter) return;

    BossCharacter->GetCharacterMovement()->GravityScale = 3.0f;

    // 2. 목표 지점(플레이어) 계산
    FVector StartLoc = BossCharacter->GetActorLocation();
    FVector TargetLoc = StartLoc; // Default

    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(BossCharacter->GetWorld(), 0))
    {
        TargetLoc = Player->GetActorLocation();
    }

    FVector Dir = (TargetLoc - StartLoc).GetSafeNormal();

    // 낙하 속도 데이터 사용
    float FallSpeed = 6000.0f;
    FVector SmashVel = Dir * FallSpeed;

    BossCharacter->LaunchCharacter(SmashVel, true, true);
    BossCharacter->LandedDelegate.AddDynamic(this, &UBossTask_Takedown::OnCharacterLanded);
}

void UBossTask_Takedown::OnCharacterLanded(const FHitResult& Hit)
{
    //UE_LOG(LogBossTest, Error, TEXT("land Enter"));
    if (InternalState != ETakedownState::Jumping) return;

    // 델리게이트 해제
    BossCharacter->LandedDelegate.RemoveDynamic(this, &UBossTask_Takedown::OnCharacterLanded);

    InternalState = ETakedownState::Recovering;

    // 물리 복구
    BossCharacter->GetCharacterMovement()->GravityScale = 1.0f;
    BossCharacter->GetCharacterMovement()->StopMovementImmediately();

    // 공격 판정
    PerformImpactLogic();

    // Land 몽타주 재생
    UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance();
    if (AnimInst && Montage_Land)
    {
        AnimInst->Montage_Play(Montage_Land);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UBossTask_Takedown::OnLandMontageEnded);
        AnimInst->Montage_SetEndDelegate(EndDelegate, Montage_Land);
    }
    else
    {
        InternalState = ETakedownState::Finished;
    }
}

void UBossTask_Takedown::PerformImpactLogic()
{
    TakedownEffect();
    // 여기에 OverlapMultiByChannel 등 데미지 로직 구현
    // DrawDebugSphere(BossCharacter->GetWorld(), BossCharacter->GetActorLocation(), 300.f, 32, FColor::Red, false, 2.0f);
}

void UBossTask_Takedown::OnLandMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    //UE_LOG(LogBossTest, Error, TEXT("last Enter"));
    InternalState = ETakedownState::Finished;
}

EStateTreeRunStatus UBossTask_Takedown::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    Super::Tick(Context, DeltaTime);

    //// ---------------------------------------------------------------
    //// [디버그 로그] 현재 상태를 문자열로 변환해서 출력
    //// ---------------------------------------------------------------
    //FString StateStr = TEXT("Unknown");
    //switch (InternalState)
    //{
    //case ETakedownState::Preparing:  StateStr = TEXT("Preparing (몽타주 재생 중)"); break;
    //case ETakedownState::Jumping:    StateStr = TEXT("Jumping (상승/체공/낙하)"); break;
    //case ETakedownState::Recovering: StateStr = TEXT("Recovering (착지 후 복구)"); break;
    //case ETakedownState::Finished:   StateStr = TEXT("Finished (종료 대기)"); break;
    //}

    //// Warning 색상(노란색)으로 출력해서 눈에 잘 띄게 함
    //// 로그 내용: [BossTask] State: Jumping ...
    //UE_LOG(LogBossTest, Warning, TEXT("[BossTask] Current State: %s"), *StateStr);


    if (InternalState == ETakedownState::Finished)
    {

        //UE_LOG(LogBossTest, Error, TEXT("finished Enter"));
        return EStateTreeRunStatus::Succeeded;
    }

    if (InternalState != ETakedownState::Recovering && BossCharacter && AttackData)
    {
        ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
        if (Player)
        {
            FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(BossCharacter->GetActorLocation(), Player->GetActorLocation());
            TargetRot = FRotator(0.0f, TargetRot.Yaw, 0.0f);
            FRotator NewRot = FMath::RInterpTo(BossCharacter->GetActorRotation(), TargetRot, DeltaTime, AttackData->RotationSpeed);
            BossCharacter->SetActorRotation(NewRot);
        }
    }

    return EStateTreeRunStatus::Running;
}

void UBossTask_Takedown::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    // [중요] 부모 ExitState 호출
    Super::ExitState(Context, Transition);
    //UE_LOG(LogBossTest, Error, TEXT("exit Enter"));

    if (BossCharacter)
    {
        TArray<UActorComponent*> Components;
        BossCharacter->GetComponents(UParticleSystemComponent::StaticClass(), Components);
        // 안전장치: 델리게이트 해제
        BossCharacter->LandedDelegate.RemoveDynamic(this, &UBossTask_Takedown::OnCharacterLanded);

        for (UActorComponent* Comp : Components)
        {
            if (Comp->ComponentHasTag(FName("TakedownBooster")))
            {
                Comp->DestroyComponent();
            }
        }

        if (UWorld* World = BossCharacter->GetWorld())
        {
            World->GetTimerManager().ClearTimer(TimerHandle_Hover);
            World->GetTimerManager().ClearTimer(TimerHandle_Smash);
        }

        // 중력 복구
        if (BossCharacter->GetCharacterMovement())
        {
            BossCharacter->GetCharacterMovement()->GravityScale = 1.0f;
        }

        // 재생 중인 몽타주 정리
        if (UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance())
        {
            if (AnimInst->Montage_IsPlaying(Montage_Start)) AnimInst->Montage_Stop(0.2f, Montage_Start);
            if (AnimInst->Montage_IsPlaying(Montage_Loop)) AnimInst->Montage_Stop(0.2f, Montage_Loop);
            if (AnimInst->Montage_IsPlaying(Montage_Land)) AnimInst->Montage_Stop(0.2f, Montage_Land);
        }
    }
}

// 직접 짠 함수
void UBossTask_Takedown::TakedownEffect()
{
    FVector SpawnLocation = BossCharacter->GetActorLocation();

    if (TakedownVFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            TakedownVFX,
            SpawnLocation
        );
    }

}