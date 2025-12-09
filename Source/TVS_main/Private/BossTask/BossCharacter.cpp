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
    // 나 자신이나, 이미 죽은 애는 무시
    if (OtherActor == this || !OtherActor) return;

    // 플레이어인지 확인 (Tag나 Class로 체크)
    if (OtherActor->ActorHasTag("Player"))
    {
        // 3. 데미지 적용! (여기서 10.0f는 데미지 양)
        //UGameplayStatics::ApplyDamage(OtherActor, 10.0f, GetController(), this, UDamageType::StaticClass());

        // (선택) 타격 이펙트/사운드 재생 코드 추가 가능
        //UE_LOG(LogTemp, Warning, TEXT("Player Hit!"));
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

    // ✅ [변경] 배열(Array)에 들어있는 개수만큼 반복합니다.
    // "리스트에 있는 거 하나씩 다 꺼내봐(Type)"
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
            // 돌진은 보통 전신 무기니까 필요하면 팔다리도 여기서 같이 켜줘도 됨
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
    // 1. 데이터가 없거나, 쏠 투사체(BP)가 설정 안 되어 있으면 취소
    if (!CurrentAttackData || !CurrentAttackData->ProjectileClass) return;

    // 2. 발사 위치(Socket) 찾기
    FVector MuzzleLoc = GetActorLocation(); // 소켓 못 찾으면 몸통 위치

    // 데이터 에셋에 적힌 소켓 이름("Muzzle_01")으로 위치 찾기
    if (!CurrentAttackData->MuzzleSocketName.IsNone())
    {
        MuzzleLoc = GetMesh()->GetSocketLocation(CurrentAttackData->MuzzleSocketName);
    }

    FRotator SpawnRotation;
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    //if (Player)
    //{
    //    // 시작점(총구)에서 목표점(플레이어)을 바라보는 회전값 계산
    //    // (플레이어의 캡슐 중심보다 약간 위나 Mesh 위치를 노리면 더 정확합니다)
    //    FVector TargetLocation = Player->GetActorLocation();

    //    SpawnRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLoc, TargetLocation);
    //}
    //else
    //{
        // 플레이어가 없으면 그냥 보스 정면으로 발사
    SpawnRotation = GetActorRotation();
    //}

    float FRand = CurrentAttackData->FRand;
    SpawnRotation.Pitch += FMath::FRandRange(-FRand, FRand);
    SpawnRotation.Yaw += FMath::FRandRange(-FRand, FRand);

    // 4. 진짜 생성 (Spawn)
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