#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BossAttackData.generated.h"

UENUM(BlueprintType)
enum class EBossPatternCategory : uint8
{
    None        UMETA(DisplayName = "없음"),
    Melee       UMETA(DisplayName = "근접 공격"),
    Charge      UMETA(DisplayName = "돌진/추격"),
    Ranged      UMETA(DisplayName = "원거리"),
    Takedown    UMETA(DisplayName = "공중찍기"),
    Stump       UMETA(DisplayName = "찍기"),
    Special     UMETA(DisplayName = "특수 패턴")
};

// 어떤 콜라이더를 쓸지 결정하는 열거형
UENUM(BlueprintType)
enum class EAttackHitboxType : uint8
{
    None,
    RightHand,
    LeftHand,
    BodyCharge,
    Slam
};

UENUM(BlueprintType)
enum class ETargetSideRequirement : uint8
{
    Any     UMETA(DisplayName = "상관없음"), // 기본값
    Left    UMETA(DisplayName = "보스 기준 왼쪽"),
    Right   UMETA(DisplayName = "보스 기준 오른쪽"),
    Back    UMETA(DisplayName = "등 뒤 (후방)")
};

/**
 * 공격 하나에 대한 모든 정보를 담는 데이터 통
 */
UCLASS(BlueprintType)
class TVS_MAIN_API UBossAttackData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    // 1. 행동 카테고리 (이 값에 따라 State Tree가 다른 가지를 탑니다)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    EBossPatternCategory PatternCategory = EBossPatternCategory::Melee;

    // 2. 콤보 연계 (이 기술 다음에 올 수 있는 기술들)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic|Combo")
    TArray<UBossAttackData*> LinkedPatterns;

    // ==========================================
    // 1. 패턴 선정 가중치 (확률)
    // ==========================================
    // 높을수록 이 조건이 맞을 때 더 자주 선택됨
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    float SelectionWeight = 1.0f;

    // ==========================================
    // 2. HP 페이즈 조건 (0.0 ~ 1.0)
    // ==========================================
    // 예: 0.5 ~ 1.0 이면 체력 50% 이상일 때만 사용 (1페이즈)
    // 예: 0.0 ~ 0.5 이면 체력 50% 이하일 때만 사용 (2페이즈 광폭화)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    float MinHPPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    float MaxHPPercent = 1.0f;

    // ==========================================
    // 3. 거리 조건 (cm)
    // ==========================================
    // 플레이어와의 거리가 이 사이일 때만 사용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    float MinRange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    float MaxRange = 1000.0f;

    // ==========================================
    // 4. 방향 조건 (각도, Degree)
    // ==========================================
    // 보스 정면 기준 몇 도(±) 안에 플레이어가 있어야 하는가?
    // 예: 45도 = 정면 부채꼴, 180도 = 전방향, 
    // 특수: -1.0 등으로 설정해서 '뒤잡기 패턴' 로직을 짤 수도 있음
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    float RequiredHitAngle = 180.0f; // 기본값은 넉넉하게

    // ==========================================
    // 1. 이전 패턴 (연속 사용 제한)
    // ==========================================
    // "이 기술을 연속으로 써도 되는가?"
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    bool bCanUseConsecutively = false;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    float CooldownTime = 0.0f; // 0이면 쿨타임 없음


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Logic")
    ETargetSideRequirement RequiredSide = ETargetSideRequirement::Any;






    // 1. 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float PlayRate = 1.0f;

    //피격 판정 설정 (다중 선택 가능)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    TArray<EAttackHitboxType> ActiveHitboxes;

    // 3. 연출 (VFX / SFX)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
    UParticleSystem* AttackVFX; // 공격 시 터지는 이펙트

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
    FName VFXSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
    USoundBase* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
    TSubclassOf<UCameraShakeBase> CameraShake;

    UPROPERTY(EditAnywhere, Category = "Combat")
    bool bEnableRotation = true;

    // 회전 속도
    UPROPERTY(EditAnywhere, Category = "Combat")
    float RotationSpeed = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Range", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Ranged", EditConditionHides))
    float FireCoolTime;

    UPROPERTY(EditAnywhere, Category = "Range", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Ranged", EditConditionHides))
    float MoveSpeed = 600.0f;

    // [추가 1] 쏠 투사체 블루프린트 (여기에 BP_BossBullet을 넣게 됨)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Ranged", EditConditionHides))
    TSubclassOf<AActor> ProjectileClass;

    // [추가 2] 총알이 나갈 소켓 이름 (예: "Muzzle_01")
    // 이펙트 소켓이랑 다를 수 있어서 따로 파는 게 좋습니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Ranged", EditConditionHides))
    FName MuzzleSocketName;
    // (필요하면 데칼 정보 등 추가)

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Ranged", EditConditionHides))
    float FRand = 1.0f;


    //Takedown 패턴용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Takedown", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Takedown", EditConditionHides))
    float JumpHeight = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Takedown", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Takedown", EditConditionHides))
    float HoverDuration = 1.0f; // 공중에서 째려보는 시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Takedown", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Takedown", EditConditionHides))
    float SlamTimeToReach = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Takedown", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Takedown", EditConditionHides))
    float DamageRadius = 500.0f;


    //Stump 패턴용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stump", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Stump", EditConditionHides))
    float StumpPlayRate = 2.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stump", meta = (EditCondition = "PatternCategory == EBossPatternCategory::Stump", EditConditionHides))
    float StumpApexTime = 2.0;

};