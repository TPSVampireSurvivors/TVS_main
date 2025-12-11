#include "JumpEnemyActor.h"
#include "EnemyActor.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"   
#include "SurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"

// Sets default values
AJumpEnemyActor::AJumpEnemyActor()
{
    PrimaryActorTick.bCanEverTick = true;

    boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
    SetRootComponent(boxComp);
    meshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    meshComp->SetupAttachment(boxComp);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("/Game/Characters/Mannequins/ALLANIMATIONS.ALLANIMATIONS")
    );

    if (MeshAsset.Succeeded())
    {
        meshComp->SetSkeletalMesh(MeshAsset.Object);
    }

    // 메시가 너무 크거나 작으면 위치/스케일 조정
    meshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    meshComp->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
    boxComp->SetCollisionProfileName(TEXT("Enemy"));
}

void AJumpEnemyActor::BeginPlay()
{
    Super::BeginPlay();
    boxComp->OnComponentBeginOverlap.AddDynamic(this, &AJumpEnemyActor::OnEnemyOverlap);
}

void AJumpEnemyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!player) return;

    FVector MyLoc = GetActorLocation();
    FVector PlayerLoc = player->GetActorLocation();

    float Dist = FVector::Dist2D(MyLoc, PlayerLoc);

    // ------------------------------
    // 1. 점프 중 포물선 이동
    // ------------------------------
    if (bIsJumping)
    {
        elapsedJumpTime += DeltaTime;
        FVector Gravity(0, 0, -980.f);

        FVector NewPos =
            jumpStart +
            jumpVelocity * elapsedJumpTime +
            0.5f * Gravity * elapsedJumpTime * elapsedJumpTime;

        SetActorLocation(NewPos);

        if (elapsedJumpTime >= jumpDuration)
        {
            bIsJumping = false;
            elapsedJumpTime = 0.0f;
        }

        return;
    }

    // ------------------------------
    // 2. 점프 준비 중
    // ------------------------------
    if (bIsPreparingJump)
    {
        return;
    }

    // ------------------------------
    // 3. 일정 거리 접근 → 점프 준비
    // ------------------------------
    if (Dist < 500.f)
    {
        bIsPreparingJump = true;

        GetWorld()->GetTimerManager().SetTimer(
            PrepareTimerHandle,
            [this]()
            {
                bIsPreparingJump = false;
                bIsJumping = true;

                jumpStart = GetActorLocation();

                ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
                if (!player)
                {
                    bIsJumping = false;     // 점프 중지
                    return;                 // 크래시 방지
                }
                FVector Target = player->GetActorLocation() + FVector(0, 0, 140.f);

                FVector Gravity(0, 0, -980.f);
                float t = jumpDuration;

                jumpVelocity = (Target - jumpStart - 0.5f * Gravity * t * t) / t;
            },
            jumpPrepareTime,
            false
        );

        return;
    }

    // ------------------------------
    // 4. 평소 이동
    // ------------------------------
    FVector Dir = PlayerLoc - MyLoc;
    Dir.Z = 0;
    Dir.Normalize();

    SetActorLocation(MyLoc + Dir * moveSpeed * DeltaTime);
}
void AJumpEnemyActor::OnEnemyOverlap(UPrimitiveComponent* OverlappedComponent, AActor*
    OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const
    FHitResult& SweepResult)
{
    ASurvivalCharacter* player = Cast<ASurvivalCharacter>(OtherActor);

    // 만일, 캐스팅이 성공했다면 ….
    if (player != nullptr)
    {
        player->TakeEnemyDamage(1);
    }
    Destroy();
}