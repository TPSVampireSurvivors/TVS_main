


#include "BossTask/BossAIController.h"
#include "BossTask/BossAttackData.h" 
#include "BossTask/BossCharacter.h"  
#include "TVS_main.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ABossAIController::ABossAIController()
{
    LastUsedPattern = nullptr;
}

void ABossAIController::ResetCombo()
{
    LastUsedPattern = nullptr;
}

bool ABossAIController::IsComboActive() const
{
    return LastUsedPattern != nullptr;
}

UBossAttackData* ABossAIController::SelectAttackPattern(const TArray<UBossAttackData*>& Candidates)
{
    //UE_LOG(LogBossTest, Warning, TEXT("AI Select: %d"), Candidates.Num());
    if (Candidates.Num() == 0) return nullptr;

    APawn* OwnerPawn = GetPawn();
    ABossCharacter* Boss = Cast<ABossCharacter>(OwnerPawn);
    AActor* Target = UGameplayStatics::GetPlayerCharacter(this, 0);

    if (!Boss || !Target) return nullptr;

    float Dist = FVector::Distance(Boss->GetActorLocation(), Target->GetActorLocation());

    FVector ToTarget = (Target->GetActorLocation() - Boss->GetActorLocation()).GetSafeNormal();
    float Dot = FVector::DotProduct(Boss->GetActorForwardVector(), ToTarget);
    float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

    FVector BossForward = Boss->GetActorForwardVector();
    FVector BossRight = Boss->GetActorRightVector();

    float RightDot = FVector::DotProduct(BossRight, ToTarget);

    TArray<UBossAttackData*> ValidPool;
    float TotalWeight = 0.0f;

    for (UBossAttackData* Data : Candidates)
    {
        if (!Data) continue;

        if (Data->MinHPPercent < Boss->CurrentHealth && Data->MaxHPPercent > Boss->CurrentHealth) continue;)
        if (Dist < Data->MinRange || Dist > Data->MaxRange) continue;
        if (Angle > Data->RequiredHitAngle) continue;
        if (Data->RequiredSide == ETargetSideRequirement::Left && RightDot > 0.0f) continue;
        if (Data->RequiredSide == ETargetSideRequirement::Right && RightDot < 0.0f) continue;
        if (!CanUseAttack(Data)) continue;

        ValidPool.Add(Data);
        TotalWeight += Data->SelectionWeight;
    }

    //UE_LOG(LogBossTest, Warning, TEXT("AI Select: final %d"), ValidPool.Num());

    if (ValidPool.Num() == 0) return nullptr;

    float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentSum = 0.0f;

    for (UBossAttackData* Data : ValidPool)
    {
        CurrentSum += Data->SelectionWeight;
        if (RandomPoint <= CurrentSum)
        {
            LastUsedPattern = Data;
            StartCooldown(Data);
            return Data;
        }
    }


    StartCooldown(ValidPool.Last());
    LastUsedPattern = ValidPool.Last();
    return ValidPool.Last();
}

bool ABossAIController::CanUseAttack(UBossAttackData* AttackData)
{
    if (!AttackData) return false;

    // 1. 장부에 기록된 적이 없으면? -> 사용 가능 (한 번도 안 썼으니까)
    if (!AttackHistory.Contains(AttackData))
    {
        return true;
    }

    // 2. 마지막 사용 시간 가져오기
    double LastUsedTime = AttackHistory[AttackData];

    // 3. 현재 시간 가져오기
    double CurrentTime = GetWorld()->GetTimeSeconds();

    // 4. (현재 시간 - 마지막 시간)이 쿨타임보다 크면 사용 가능!
    if ((CurrentTime - LastUsedTime) >= AttackData->CooldownTime)
    {
        return true;
    }

    // 아직 쿨타임 안 돌았음
    return false;
}

void ABossAIController::StartCooldown(UBossAttackData* AttackData)
{
    if (!AttackData) return;

    // 현재 시간을 장부에 기록 (덮어쓰기)
    AttackHistory.Add(AttackData, GetWorld()->GetTimeSeconds());
}