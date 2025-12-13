// Fill out your copyright notice in the Description page of Project Settings.

#include "BossTask/BossCharacter.h"
#include "BossTask/BossAttackData.h"
#include "BossTask/BossAIController.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/KismetMathLibrary.h>


// Sets default values
ABossCharacter::ABossCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // [핵심] 이 보스가 스폰될 때, 기본으로 사용할 AI 컨트롤러 클래스를 지정합니다.
    AIControllerClass = ABossAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

    // 1. 오른손 박스
    RightFistBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightFistBox"));
    RightFistBox->SetupAttachment(GetMesh(), TEXT("RightHandSocket"));
    RightFistBox->SetCollisionProfileName(TEXT("NoCollision"));

    // 2. 왼손 박스
    LeftFistBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftFistBox"));
    LeftFistBox->SetupAttachment(GetMesh(), TEXT("LeftHandSocket"));
    LeftFistBox->SetCollisionProfileName(TEXT("NoCollision"));

    // 3. 몸통 돌진 박스 (기본 캡슐보다 조금 더 크게 설정 추천)
    BodyChargeBox = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyChargeBox"));
    BodyChargeBox->SetupAttachment(GetRootComponent()); // 루트에 붙임
    BodyChargeBox->SetCollisionProfileName(TEXT("NoCollision"));

    // 직접 짠 코드
    SlamSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SlamSphere"));
    SlamSphere->SetupAttachment(GetMesh(), TEXT("LeftHandSocket"));
    SlamSphere->SetCollisionProfileName(TEXT("NoCollision"));

}

// Called when the game starts or when spawned
void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();
    // 이벤트 연결 (함수 하나를 여러 컴포넌트에 연결해도 됩니다!)
    RightFistBox->OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnAttackOverlap);
    LeftFistBox->OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnAttackOverlap);
    BodyChargeBox->OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnAttackOverlap);
    SlamSphere->OnComponentBeginOverlap.AddDynamic(this, &ABossCharacter::OnAttackOverlap);
	
    CurrentHealth = MaxHealth;
}

// Called every frame
void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABossCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABossCharacter::OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor == this || !OtherActor) return;

    if (OtherActor->ActorHasTag("Player"))
    {
        ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this->GetWorld(), 0);
        if (!Player) return;
        UGameplayStatics::ApplyDamage(Player, 1.0f, nullptr, nullptr, nullptr);
    }
}

// [추가] Task에서 공격 시작할 때 호출하는 함수
void ABossCharacter::InitAttack(const UBossAttackData* NewAttackData)
{
    CurrentAttackData = NewAttackData; // 1. 데이터를 기억해둠

    // 2. 몽타주 재생
    if (CurrentAttackData && CurrentAttackData->Montage)
    {
        PlayAnimMontage(CurrentAttackData->Montage, CurrentAttackData->PlayRate);
    }
}

void ABossCharacter::ActivateHitbox(bool bActive)
{
    if (!CurrentAttackData) return;

    FName NewProfile = bActive ? TEXT("OverlapAllDynamic") : TEXT("NoCollision");

    for (const EAttackHitboxType& Type : CurrentAttackData->ActiveHitboxes)
    {
        switch (Type)
        {
        case EAttackHitboxType::RightHand:
            RightFistBox->SetCollisionProfileName(NewProfile);
            break;

        case EAttackHitboxType::LeftHand:
            LeftFistBox->SetCollisionProfileName(NewProfile);
            break;

        case EAttackHitboxType::BodyCharge:
            BodyChargeBox->SetCollisionProfileName(NewProfile);
            break;

        case EAttackHitboxType::Slam:
            SlamSphere->SetCollisionProfileName(NewProfile);
            break;
        }
    }

    // 이펙트는 한 번만
    if (bActive)
    {
        ExecuteAttackEffect();
    }
}

// [추가] 이펙트 재생 전용 함수
void ABossCharacter::ExecuteAttackEffect()
{
    // 1. 기본 위치는 보스의 발밑(ActorLocation)으로 잡습니다.
    FVector SpawnLocation = GetActorLocation();
    //FRotator SpawnRotation = GetActorRotation(); // 회전도 보스 방향 따라가게

    // 2. 만약 데이터 에셋에 '소켓 이름'이 적혀 있다면? 그 위치로 덮어씁니다.
    if (!CurrentAttackData->VFXSocketName.IsNone())
    {
        // 메쉬한테 "이 이름 가진 소켓 위치 내놔" 라고 물어봅니다.
        SpawnLocation = GetMesh()->GetSocketLocation(CurrentAttackData->VFXSocketName);

        // (필요하다면) 회전도 소켓 회전을 따라갈 수 있습니다.
        // SpawnRotation = GetMesh()->GetSocketRotation(CurrentAttackData->VFXSocketName);
    }

    // 3. (선택 사항) 오프셋 더하기 (조금 앞, 조금 위 등 미세 조정)
    //SpawnLocation += CurrentAttackData->VFXOffset;
        

    // 4. 최종 위치에 이펙트 쾅!
    if (CurrentAttackData->AttackVFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            CurrentAttackData->AttackVFX,
            SpawnLocation
            //SpawnRotation // 혹은 FRotator::ZeroRotator (회전 무시하고 싶으면)
        );
    }

    // 사운드도 같은 위치에서 나게 하면 리얼하겠죠?
    if (CurrentAttackData->AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CurrentAttackData->AttackSound, SpawnLocation);
    }

    // 3. 카메라 셰이크
    if (CurrentAttackData->CameraShake)
    {
        UGameplayStatics::PlayWorldCameraShake(GetWorld(), CurrentAttackData->CameraShake, GetActorLocation(), 0, 1000, 1);
    }
}

void ABossCharacter::FireProjectile(FRotator FireRotation)
{
    if (!CurrentAttackData || !CurrentAttackData->ProjectileClass) return;

    FVector MuzzleLoc = GetActorLocation(); // 소켓 못 찾으면 몸통 위치

    if (!CurrentAttackData->MuzzleSocketName.IsNone())
    {
        MuzzleLoc = GetMesh()->GetSocketLocation(CurrentAttackData->MuzzleSocketName);
    }

    FRotator SpawnRotation;
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    SpawnRotation = GetActorRotation();

    float FRand = CurrentAttackData->FRand;
    SpawnRotation.Pitch += FMath::FRandRange(-FRand, FRand);
    SpawnRotation.Yaw += FMath::FRandRange(-FRand, FRand);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;      // 총알 주인은 나 (팀킬 방지용)
    SpawnParams.Instigator = this; // 가해자도 나

    GetWorld()->SpawnActor<AActor>(
        CurrentAttackData->ProjectileClass, // 데이터 에셋에 넣은 BP
        MuzzleLoc,                          // 소켓 위치
        SpawnRotation,                           // 계산된 회전
        SpawnParams
    );

    // (선택) 발사음 재생
    if (CurrentAttackData->AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CurrentAttackData->AttackSound, MuzzleLoc);
    }
}

