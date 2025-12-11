#include "EnemyActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "SurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
// Sets default values
AEnemyActor::AEnemyActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	SetRootComponent(boxComp);
	boxComp->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static mesh"));
	meshComp->SetupAttachment(boxComp);

	boxComp->SetCollisionProfileName(TEXT("Enemy"));
}

// Called when the game starts or when spawned
void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	
	if (player == nullptr)
	{
		dir = FVector::ZeroVector;
		return;
	}

	int32 drawResult = FMath::RandRange(1, 100);

	if (drawResult <= traceRate)
	{
		dir = player->GetActorLocation() - GetActorLocation();
		dir.Normalize();
	}
	else
	{
		dir = GetActorForwardVector();
	}
	boxComp->OnComponentBeginOverlap.AddDynamic(this, &AEnemyActor::OnEnemyOverlap);
}

// Called every frame
void AEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (!player)
		return;

	dir = player->GetActorLocation() - GetActorLocation();
	dir.Normalize();
	FVector newLocation = GetActorLocation() + dir * moveSpeed * DeltaTime;
	SetActorLocation(newLocation);
}
void AEnemyActor::OnEnemyOverlap(UPrimitiveComponent* OverlappedComponent, AActor*
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