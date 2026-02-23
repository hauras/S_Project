#include "Actor/Map/RoomBase.h"
#include "Actor/Map/DungeonGenerator.h"
#include "Components/BoxComponent.h"
#include "Character/EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"

// =============================================================================
// 1. 생성자 및 초기화 (Lifecycle)
// =============================================================================

ARoomBase::ARoomBase()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>("TriggerBox");
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(2000.f, 2000.f, 500.f));
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

// [중요] 생성기에서 로드가 완료된 후 딱 한 번 호출됨
void ARoomBase::CacheInternalActors()
{
	ULevel* MyLevel = GetLevel();
	if (!MyLevel) return;

	for (AActor* Actor : MyLevel->Actors)
	{
		if (!Actor) continue;

		// 문 캐싱
		if (Actor->ActorHasTag(FName("NorthGate"))) NorthGates.Add(Actor);
		else if (Actor->ActorHasTag(FName("SouthGate"))) SouthGates.Add(Actor);
		else if (Actor->ActorHasTag(FName("EastGate")))  EastGates.Add(Actor);
		else if (Actor->ActorHasTag(FName("WestGate")))  WestGates.Add(Actor);

		// 스폰 지점 캐싱
		else if (Actor->ActorHasTag(FName("MeleePoint")))  MeleePoints.Add(Actor);
		else if (Actor->ActorHasTag(FName("RangedPoint"))) RangedPoints.Add(Actor);
	}
}

// =============================================================================
// 2. 플레이어 진입 및 전투 시작 (Combat Start)
// =============================================================================

void ARoomBase::OnPlayerEntered(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	
	// 플레이어(Pawn)가 들어왔을 때만 실행
	if (OtherActor && OtherActor->IsA(APawn::StaticClass()))
	{
		// 트리거는 한 번만 작동하도록 끔
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		UE_LOG(LogTemp, Warning, TEXT("Room [%s]: Player Entered!"), *MyGridLocation.ToString());

		CloseAllGates(); // 1. 문 닫기
		SpawnEnemy();    // 2. 적 소환
	}
}

void ARoomBase::CloseAllGates()
{
	// 모든 방향의 문 리스트를 하나로 합쳐서 한 번에 닫음
	TArray<TArray<AActor*>*> AllGateLists = { &NorthGates, &SouthGates, &EastGates, &WestGates };

	for (TArray<AActor*>* GateList : AllGateLists)
	{
		for (AActor* Gate : *GateList)
		{
			if (Gate)
			{
				Gate->SetActorHiddenInGame(false);
				Gate->SetActorEnableCollision(true);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Room Locked!"));
}

void ARoomBase::SpawnEnemy()
{
	MonsterCount = 0;

	// [최적화] 전체 액터 순회 대신 미리 찾아둔 포인트 배열만 사용
	auto SpawnLogic = [&](TArray<AActor*>& Points, TSubclassOf<APawn> ClassToSpawn) {
		for (AActor* Point : Points)
		{
			if (!Point) continue;
			
			AEnemyCharacter* NewMonster = GetWorld()->SpawnActor<AEnemyCharacter>(
				ClassToSpawn, Point->GetActorLocation(), Point->GetActorRotation()
			);

			if (NewMonster)
			{
				MonsterCount++;
				NewMonster->MyRoom = this;
			}
		}
	};

	SpawnLogic(MeleePoints, MeleeMonster);
	SpawnLogic(RangedPoints, RangedMonster);
	
	// 소환된 적이 없으면 즉시 문을 열어줌
	if (MonsterCount <= 0)
	{
		OpenDoors();
	}
}

// 3. 전투 종료 및 문 열기 (Combat End)
void ARoomBase::EnemyDied()
{
	MonsterCount--;

	if (MonsterCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Room Cleared!"));
		
		// [최적화] GetActorOfClass 대신 저장해둔 MyGenerator 바로 사용
		if (MyGenerator)
		{
			MyGenerator->NotifyNeighborDoors(MyGridLocation, MyDoorBitmask);
		}
	}
}

void ARoomBase::OpenDoors()
{
	if (!HasAuthority()) return;

	// 특정 방향 리스트의 문을 여는 헬퍼 함수
	auto OpenGateList = [](TArray<AActor*>& GateList) {
		for (AActor* Gate : GateList)
		{
			if (Gate)
			{
				Gate->SetActorHiddenInGame(true);
				Gate->SetActorEnableCollision(false);
			}
		}
	};

	// 비트마스크를 체크하여 연결된 통로만 열기
	if (MyDoorBitmask & 1) OpenGateList(NorthGates);
	if (MyDoorBitmask & 2) OpenGateList(SouthGates);
	if (MyDoorBitmask & 4) OpenGateList(WestGates);
	if (MyDoorBitmask & 8) OpenGateList(EastGates);
}