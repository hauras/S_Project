#include "Actor/Map/RoomBase.h"

#include "Actor/Map/DungeonGenerator.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Character/EnemyCharacter.h" // 몬스터 클래스 포함 필수
#include "Kismet/GameplayStatics.h"

ARoomBase::ARoomBase()
{
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 2. 트리거 박스 생성 및 부착
	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerBox");
	TriggerBox->SetupAttachment(SceneRoot);
    
	// 3. 트리거 크기 설정 (예: 4400방 기준이면 절반인 2200 정도로 넉넉하게)
	TriggerBox->SetBoxExtent(FVector(2000.f, 2000.f, 500.f));

	// 4. 충돌 설정: 오직 겹침(Overlap)만 허용하고, 플레이어만 감지하도록 설정
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	
}

void ARoomBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARoomBase::OnPlayerEntered);
	}
}

void ARoomBase::OnPlayerEntered(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	
	if (OtherActor && OtherActor->IsA(APawn::StaticClass()))
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UE_LOG(LogTemp, Warning, TEXT("Combat Started in Room!"));

		CloseAllGates(); 

		SpawnEnemy(); 

	}
}

void ARoomBase::OpenDoors()
{
	if (!HasAuthority()) return;

	ULevel* MyLevel = GetLevel();
	if (!MyLevel) return;

	for (AActor* Actor : MyLevel->Actors)
	{
		if (!Actor) continue;

		// 비트마스크 상 '길이 있는 방향'의 문만 골라서 엽니다.
		if (((MyDoorBitmask & 1) && Actor->ActorHasTag(FName("NorthGate"))) ||
			((MyDoorBitmask & 2) && Actor->ActorHasTag(FName("SouthGate"))) ||
			((MyDoorBitmask & 4) && Actor->ActorHasTag(FName("WestGate")))  ||
			((MyDoorBitmask & 8) && Actor->ActorHasTag(FName("EastGate"))))
		{
			// [수정] Destroy 대신 숨기고 통과 가능하게 만듦
			Actor->SetActorHiddenInGame(true);
			Actor->SetActorEnableCollision(false);
		}
	}
}


void ARoomBase::SpawnEnemy()
{
	if (!HasAuthority()) return;

	ULevel* MyLevel = GetLevel();
	if (!MyLevel) return;

	MonsterCount = 0; // 카운트 초기화

	for (AActor* Actor : MyLevel->Actors)
	{
		if (!Actor) continue;

		FVector SpawnLocation = Actor->GetActorLocation();
		AEnemyCharacter* NewMonster = nullptr;

		// 1. 근거리 포인트 체크
		if (Actor->ActorHasTag(FName("MeleePoint")))
		{
			NewMonster = GetWorld()->SpawnActor<AEnemyCharacter>(MeleeMonster, SpawnLocation, FRotator::ZeroRotator);
		}
		// 2. 원거리 포인트 체크
		else if (Actor->ActorHasTag(FName("RangedPoint")))
		{
			NewMonster = GetWorld()->SpawnActor<AEnemyCharacter>(RangedMonster, SpawnLocation, FRotator::ZeroRotator);
		}

		// 3. 스폰 성공 시 몬스터에게 방 정보 전달
		if (NewMonster)
		{
			MonsterCount++;
			NewMonster->MyRoom = this; // 몬스터에게 "니 주인은 나야"라고 알려줌
		}
	}
	
	// 만약 포인트는 있는데 몬스터가 하나도 스폰 안 됐다면 즉시 문을 열어줌 (예외 처리)
	if (MonsterCount <= 0)
	{
		OpenDoors();
	}

	UE_LOG(LogTemp, Warning, TEXT("Room at %s: Spawned %d Monsters"), *GetActorLocation().ToString(), MonsterCount);
}

void ARoomBase::CloseAllGates()
{
	if (!HasAuthority()) return;

	ULevel* MyLevel = GetLevel();
	for (AActor* Actor : MyLevel->Actors)
	{
		if (Actor && (Actor->ActorHasTag(FName("NorthGate")) || 
					  Actor->ActorHasTag(FName("SouthGate")) || 
					  Actor->ActorHasTag(FName("WestGate"))  || 
					  Actor->ActorHasTag(FName("EastGate"))))
		{
			// 문을 보이게 하고 길을 막습니다. (닫힘)
			Actor->SetActorHiddenInGame(false);
			Actor->SetActorEnableCollision(true);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Room Locked! All gates closed."));
}

void ARoomBase::EnemyDied()
{
	MonsterCount--;
	if (MonsterCount <= 0)
	{
		// 직접 OpenDoors를 부르는 대신, 생성기에게 보고해서 옆 방 문까지 같이 열게 합니다.
		ADungeonGenerator* Generator = Cast<ADungeonGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonGenerator::StaticClass()));
		if (Generator)
		{
			Generator->NotifyNeighborDoors(MyGridLocation, MyDoorBitmask);
		}
	}
}
