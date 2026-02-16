#include "Actor/Map/DungeonGenerator.h"
#include "Net/UnrealNetwork.h"
#include "Engine/LevelStreamingDynamic.h"

ADungeonGenerator::ADungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = true; // Tick을 사용하므로 true
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
	RoomMap.Empty();
	DungeonLayout.Empty();

	const FIntPoint Directions[] = {
		FIntPoint(0,1), FIntPoint(0,-1), FIntPoint(-1,0), FIntPoint(1,0)
	};

	TQueue<FIntPoint> Queue;

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

			if (!RoomMap.Contains(NextPos) && RoomMap.Num() < MaxRooms && FMath::FRand() < 0.8f) 
			{
				RoomMap[CurrentPos].DoorBitmask |= (1 << i);

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

	AssignSpecialRooms();
	SpawnDungeon();

	UE_LOG(LogTemp, Warning, TEXT("Dungeon Generated! Total Rooms: %d"), DungeonLayout.Num());
}

void ADungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 로컬 플레이어 컨트롤러를 가져옴 (클라이언트/서버 각자 자신의 화면 최적화)
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	FVector PlayerLoc = PC->GetPawn()->GetActorLocation();

	int32 GridX = FMath::RoundToInt(PlayerLoc.X / RoomSize);
	int32 GridY = FMath::RoundToInt(PlayerLoc.Y / RoomSize);
	FIntPoint CurrentGridPos(GridX, GridY);

	if (CurrentGridPos != LastPlayerGridPos)
	{
		LastPlayerGridPos = CurrentGridPos;
		UpdateRoomVisibility(CurrentGridPos);
	}
}

void ADungeonGenerator::UpdateRoomVisibility(FIntPoint CurrentGridPos)
{
	// [수정] LevelDataMap (또는 LevelBitmaskMap) 순회
	// 헤더에 정의하신 이름이 LevelBitmaskMap이라면 맞춰서 사용하세요.
	for (auto& Pair : LevelDataMap) 
	{
		ULevelStreamingDynamic* LevelInstance = Pair.Key;
		FRoomData& Data = Pair.Value;

		// [수정] 맨해튼 거리 계산 (변수명 수정)
		int32 Dist = FMath::Abs(Data.GridLocation.X - CurrentGridPos.X) + FMath::Abs(Data.GridLocation.Y - CurrentGridPos.Y);

		// 거리가 1 이하인 방들만 렌더링하도록 설정합니다.
		if (Dist <= 1)
		{
			LevelInstance->SetShouldBeVisible(true);
		}
		else
		{
			LevelInstance->SetShouldBeVisible(false); // 화면에서 가림 (VRAM 절약)
		}
	}
}

void ADungeonGenerator::AssignSpecialRooms()
{
	FIntPoint BossCoords = FIntPoint(0, 0);
	int32 MaxDepth = -1;
	
	for (auto& pair : RoomMap)
	{
		if (pair.Key == FIntPoint(0,0)) continue;

		if (pair.Value.Depth > MaxDepth)
		{
			MaxDepth = pair.Value.Depth;
			BossCoords = pair.Key;
		}
	}
	
	// [수정] 루프가 끝난 후 보스방 확정
	if (RoomMap.Contains(BossCoords))
	{
		RoomMap[BossCoords].RoomType = ERoomType::Boss;
	}

	int32 TreasureCount = FMath::RoundToInt(RoomMap.Num() * TreasureRoomRatio);
	int32 CurrentTreasureCount = 0;

	for (auto& pair : RoomMap)
	{
		if (CurrentTreasureCount >= TreasureCount) break;
		
		FRoomData& Room = pair.Value;
		if (Room.RoomType == ERoomType::Normal)
		{
			int32 Mask = Room.DoorBitmask;
			if (Mask == 1 || Mask == 2 || Mask == 4 || Mask == 8)
			{
				Room.RoomType = ERoomType::Treasure;
				CurrentTreasureCount++;
			}
		}
	}

	RoomMap.GenerateValueArray(DungeonLayout);
}

void ADungeonGenerator::SpawnDungeon()
{
	if (RoomLevel.IsNull()) return;
	
	// 기존에 생성된 레벨이 있다면 장부 초기화
	LevelDataMap.Empty();

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
			LevelDataMap.Add(StreamingLevel, Data);
		}
	}
}

void ADungeonGenerator::OnRoomLevelShown()
{
	for (auto It = LevelDataMap.CreateIterator(); It; ++It)
	{
		ULevelStreamingDynamic* StreamingLevel = It.Key();
		FRoomData& Data = It.Value();

		if (StreamingLevel && StreamingLevel->IsLevelLoaded() && StreamingLevel->GetLoadedLevel())
		{
			ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel();
			
			for (AActor* Actor : LoadedLevel->Actors)
			{
				if (!Actor) continue;

				if ((Data.DoorBitmask & 1) && Actor->ActorHasTag(FName("NorthGate"))) Actor->Destroy();
				if ((Data.DoorBitmask & 2) && Actor->ActorHasTag(FName("SouthGate"))) Actor->Destroy();
				if ((Data.DoorBitmask & 4) && Actor->ActorHasTag(FName("WestGate"))) Actor->Destroy();
				if ((Data.DoorBitmask & 8) && Actor->ActorHasTag(FName("EastGate"))) Actor->Destroy();

				if (Data.RoomType != ERoomType::Treasure && Actor->ActorHasTag(FName("TreasureContent"))) Actor->Destroy();
				if (Data.RoomType != ERoomType::Boss && Actor->ActorHasTag(FName("BossContent"))) Actor->Destroy();
			}
		}
	}
}

void ADungeonGenerator::OnRep_DungeonLayout()
{
	SpawnDungeon();
}