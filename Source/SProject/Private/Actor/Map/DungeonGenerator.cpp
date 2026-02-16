#include "Actor/Map/DungeonGenerator.h"
#include "Net/UnrealNetwork.h"
#include "Actor/Map/RoomBase.h"
#include "Engine/LevelStreamingDynamic.h"

ADungeonGenerator::ADungeonGenerator()
{
	bReplicates = true;
	bAlwaysRelevant = true; 
}

void ADungeonGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADungeonGenerator, DungeonLayout);
}

void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GenerateDungeon();
	}
}

void ADungeonGenerator::GenerateDungeon()
{
	// 1. 초기화 (재생성 대비)
	RoomMap.Empty();
	DungeonLayout.Empty();

	const FIntPoint Directions[] = {
		FIntPoint(0,1), FIntPoint(0,-1), FIntPoint(-1,0), FIntPoint(1,0)
	};

	TQueue<FIntPoint> Queue;

	// 시작 방 설정
	FRoomData StartRoom;
	StartRoom.GridLocation = FIntPoint(0,0);
	StartRoom.Depth = 0;
	StartRoom.RoomType = ERoomType::Start;

	RoomMap.Add(StartRoom.GridLocation, StartRoom);
	Queue.Enqueue(StartRoom.GridLocation);

	FIntPoint CurrentPos;
	while (!Queue.IsEmpty())
	{
		Queue.Dequeue(CurrentPos); 

		for (int32 i = 0; i < 4; ++i)
		{
			FIntPoint NextPos = CurrentPos + Directions[i];

			// 생성 조건: 중복X, 최대개수 미만, 랜덤확률 (테스트를 위해 확률 높임)
			if (!RoomMap.Contains(NextPos) && RoomMap.Num() < MaxRooms && FMath::FRand() < 0.8f) 
			{
				// 부모 문 열기
				RoomMap[CurrentPos].DoorBitmask |= (1 << i);

				// 자식 방 생성 및 문 열기
				int32 OppositeIndex = (i % 2 == 0) ? i + 1 : i - 1;
				FRoomData NewRoom;
				NewRoom.GridLocation = NextPos;
				NewRoom.Depth = RoomMap[CurrentPos].Depth + 1;
				NewRoom.DoorBitmask |= (1 << OppositeIndex);

				RoomMap.Add(NextPos, NewRoom);
				Queue.Enqueue(NextPos);
			}
		}
	}

	// [핵심] 방 생성이 끝난 후 특수 방(보스, 보물)을 지정합니다.
	AssignSpecialRooms();

	// 설계도가 완성되었으므로 서버도 스폰을 시작합니다.
	SpawnDungeon();

	UE_LOG(LogTemp, Warning, TEXT("Dungeon Generated! Total Rooms: %d"), DungeonLayout.Num());
}

void ADungeonGenerator::AssignSpecialRooms()
{
	FIntPoint BossCoords = FIntPoint(0, 0);
	int32 MaxDepth = -1;
	
	// 1. 보스방 찾기 루프
	for (auto& pair : RoomMap)
	{
		if (pair.Value.Depth > MaxDepth)
		{
			MaxDepth = pair.Value.Depth;
			BossCoords = pair.Key;
		}
	}
	// 가장 먼 곳을 보스방으로 확정
	RoomMap[BossCoords].RoomType = ERoomType::Boss;

	// 2. 보물방 배정 루프
	int32 TreasureCount = FMath::RoundToInt(RoomMap.Num() * TreasureRoomRatio);
	int32 CurrentTreasureCount = 0;

	for (auto& pair : RoomMap)
	{
		if (CurrentTreasureCount >= TreasureCount) break;
		
		FRoomData& Room = pair.Value;
		// 이미 보스나 시작방이면 제외
		if (Room.RoomType != ERoomType::Normal) continue;

		// 막다른 길(문이 1개)인 경우 보물방으로 승격
		int32 Mask = Room.DoorBitmask;
		if (Mask == 1 || Mask == 2 || Mask == 4 || Mask == 8)
		{
			Room.RoomType = ERoomType::Treasure; // [수정] 타입을 실제로 변경
			CurrentTreasureCount++;
		}
	}

	// 3. 최종 설계도를 배열에 옮겨 담기 (클라 복제용)
	RoomMap.GenerateValueArray(DungeonLayout);
}

void ADungeonGenerator::SpawnDungeon()
{
	if (RoomLevel.IsNull()) return;
	
	for (const FRoomData& Data : DungeonLayout)
	{
		FVector SpawnLocation = FVector(Data.GridLocation.X * RoomSize, Data.GridLocation.Y * RoomSize, 0.f);
		FString UniqueLevelName = FString::Printf(TEXT("Room_%d_%d"), Data.GridLocation.X, Data.GridLocation.Y);

		bool bOutSuccess = false;
		ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
			GetWorld(), RoomLevel, SpawnLocation, FRotator::ZeroRotator, bOutSuccess, UniqueLevelName
		);

		if (StreamingLevel && bOutSuccess)
		{
			StreamingLevel->OnLevelShown.AddDynamic(this, &ADungeonGenerator::OnRoomLevelShown);
			// [수정] 비트마스크 숫자만 넣지 말고 방 정보 전체(Data)를 저장
			LevelDataMap.Add(StreamingLevel, Data);
		}
	}
}

void ADungeonGenerator::OnRoomLevelShown()
{
	// 로딩 완료된 레벨들을 조사
	for (auto It = LevelDataMap.CreateIterator(); It; ++It)
	{
		ULevelStreamingDynamic* StreamingLevel = It.Key();
		FRoomData Data = It.Value();

		if (StreamingLevel && StreamingLevel->IsLevelLoaded() && StreamingLevel->GetLoadedLevel())
		{
			ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel();
			
			for (AActor* Actor : LoadedLevel->Actors)
			{
				if (!Actor) continue;

				// 1. 문 제거 로직 (비트마스크)
				if ((Data.DoorBitmask & 1) && Actor->ActorHasTag(FName("NorthGate"))) Actor->Destroy();
				if ((Data.DoorBitmask & 2) && Actor->ActorHasTag(FName("SouthGate"))) Actor->Destroy();
				if ((Data.DoorBitmask & 4) && Actor->ActorHasTag(FName("WestGate"))) Actor->Destroy();
				if ((Data.DoorBitmask & 8) && Actor->ActorHasTag(FName("EastGate"))) Actor->Destroy();

				// 2. 방 종류별 시각화 (예: 보물방에만 보물상자 보이기)
				// 템플릿 레벨 안에 TreasureChest라는 태그를 가진 액터가 있다고 가정
				if (Data.RoomType != ERoomType::Treasure && Actor->ActorHasTag(FName("TreasureContent"))) Actor->Destroy();
				if (Data.RoomType != ERoomType::Boss && Actor->ActorHasTag(FName("BossContent"))) Actor->Destroy();
			}
			It.RemoveCurrent();
		}
	}
}

void ADungeonGenerator::OnRep_DungeonLayout()
{
	SpawnDungeon();
}