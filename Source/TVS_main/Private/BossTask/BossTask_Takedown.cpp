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

UBossTask_Takedown::UBossTask_Takedown(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bShouldCallTick = true;
}

EStateTreeRunStatus UBossTask_Takedown::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    Super::EnterState(Context, Transition);

    //UE_LOG(LogBossTest, Error, TEXT("Takedown Enter"));

    AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
    BossCharacter = Cast<ACharacter>(OwnerActor);

    if (!BossCharacter || !Montage_Start)
    {
        //UE_LOG(LogBossTest, Error, TEXT("Failed"));
        return EStateTreeRunStatus::Failed;
    }

    UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance();
    if (!AnimInst) return EStateTreeRunStatus::Failed;

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
        OnStartMontageEnded(Montage_Start, false);
    }

    return EStateTreeRunStatus::Running;
}

void UBossTask_Takedown::OnStartMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bInterrupted || InternalState != ETakedownState::Preparing) return;

    InternalState = ETakedownState::Jumping;

    StartAscend();

    if (UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance())
    {
        if (Montage_Loop) AnimInst->Montage_Play(Montage_Loop);
    }
}

void UBossTask_Takedown::StartAscend()
{
    if (!BossCharacter) return;

    float TargetHeight = (AttackData) ? AttackData->JumpHeight : 5000.0f;
    float ReachTime = (AttackData) ? AttackData->HoverDuration : 3.0f;

    float UpVelocity = TargetHeight / ReachTime;
    BossCharacter->GetCharacterMovement()->GravityScale = 0.0f;

    FVector LaunchVel = FVector(0, 0, UpVelocity);
    BossCharacter->LaunchCharacter(LaunchVel, true, true);

    if (AttackData && AttackData->AttackVFX)
    {
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
            PSC->ComponentTags.Add(FName("TakedownBooster"));

            FRotator BoosterRot = FRotator(180.0f, 0.0f, 0.0f);
            FVector BoosterOffset = FVector(0.0f, 0.0f, 150.0f);

            PSC->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
            PSC->SetRelativeRotation(BoosterRot);
            PSC->SetRelativeLocation(BoosterOffset);
        }
    }

    if (UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance())
    {
        if (Montage_Loop) AnimInst->Montage_Play(Montage_Loop);
    }

    BossCharacter->GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_Hover,
        this,
        &UBossTask_Takedown::StartHover,
        ReachTime,
        false
    );
}

void UBossTask_Takedown::StartHover()
{
    if (!BossCharacter) return;

    BossCharacter->GetCharacterMovement()->StopMovementImmediately();
    BossCharacter->GetCharacterMovement()->GravityScale = 0.0f;

    float HoverDuration = (AttackData) ? AttackData->HoverDuration : 3.0f;

    BossCharacter->GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_Smash,
        this,
        &UBossTask_Takedown::StartSmash,
        HoverDuration,
        false
    );
}

void UBossTask_Takedown::StartSmash()
{
    if (!BossCharacter) return;

    BossCharacter->GetCharacterMovement()->GravityScale = 3.0f;

    FVector StartLoc = BossCharacter->GetActorLocation();
    FVector TargetLoc = StartLoc; 

    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(BossCharacter->GetWorld(), 0))
    {
        TargetLoc = Player->GetActorLocation();
    }

    FVector Dir = (TargetLoc - StartLoc).GetSafeNormal();

    float FallSpeed = 6000.0f;
    FVector SmashVel = Dir * FallSpeed;

    BossCharacter->LaunchCharacter(SmashVel, true, true);
    BossCharacter->LandedDelegate.AddDynamic(this, &UBossTask_Takedown::OnCharacterLanded);
}

void UBossTask_Takedown::OnCharacterLanded(const FHitResult& Hit)
{
    if (InternalState != ETakedownState::Jumping) return;

    BossCharacter->LandedDelegate.RemoveDynamic(this, &UBossTask_Takedown::OnCharacterLanded);

    InternalState = ETakedownState::Recovering;

    BossCharacter->GetCharacterMovement()->GravityScale = 1.0f;
    BossCharacter->GetCharacterMovement()->StopMovementImmediately();

    PerformImpactLogic();

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
}

void UBossTask_Takedown::OnLandMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    //UE_LOG(LogBossTest, Error, TEXT("last Enter"))
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