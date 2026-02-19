#include "Actor/Map/DungeonGenerator.h"

#include "Actor/Map/RoomBase.h"
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

void ADungeonGenerator::NotifyNeighborDoors(FIntPoint ClearedCoords, int32 Bitmask)
{
	const FIntPoint Dirs[] = { FIntPoint(0,1), FIntPoint(0,-1), FIntPoint(-1,0), FIntPoint(1,0) };

	// 1. 현재 방 문 열기
	if (RuntimeRoomMap.Contains(ClearedCoords))
		RuntimeRoomMap[ClearedCoords]->OpenDoors();

	// 2. 연결된 이웃 방들 찾아서 문 열기
	for (int32 i = 0; i < 4; ++i)
	{
		if (Bitmask & (1 << i)) // 이 방향에 길이 있다면
		{
			FIntPoint NeighborPos = ClearedCoords + Dirs[i];
			if (RuntimeRoomMap.Contains(NeighborPos))
			{
				// 옆 방에게도 "문 열어!" 라고 시킴
				// 옆 방의 OpenDoors는 자기 비트마스크를 체크해서 나랑 마주보는 문을 정확히 열게 됨
				RuntimeRoomMap[NeighborPos]->OpenDoors();
			}
		}
	}
}

FIntPoint ADungeonGenerator::GetGridCoordsFromWorldLocation(FVector WorldLocation) const
{
	int32 GridX = FMath::RoundToInt(WorldLocation.X / RoomSize);
	int32 GridY = FMath::RoundToInt(WorldLocation.Y / RoomSize);

	return FIntPoint(GridX, GridY);
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

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	// 함수를 사용하여 현재 플레이어의 그리드 위치를 한 줄로 가져옴
	FIntPoint CurrentGridPos = GetGridCoordsFromWorldLocation(PC->GetPawn()->GetActorLocation());

	if (CurrentGridPos != LastPlayerGridPos)
	{
		LastPlayerGridPos = CurrentGridPos;
		UpdateRoomVisibility(CurrentGridPos);
	}
}

void ADungeonGenerator::UpdateRoomVisibility(FIntPoint CurrentGridPos)
{
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
	// 델리게이트에 의해 호출될 때마다 전체 장부를 검사합니다.
	for (auto It = LevelDataMap.CreateIterator(); It; ++It)
	{
		ULevelStreamingDynamic* StreamingLevel = It.Key();
		FRoomData& Data = It.Value();

		// 로딩이 완료되었고, 아직 '문 지우기' 처리를 안 한 레벨만 골라냅니다.
		// (Data.Depth를 -1로 만드는 식으로 처리 완료 표시를 할 수 있습니다.)
		if (StreamingLevel && StreamingLevel->IsLevelLoaded() && StreamingLevel->GetLoadedLevel() && Data.Depth != -999)
		{
			ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel();
			ARoomBase* Room = nullptr;
			
			for (AActor* Actor : LoadedLevel->Actors)
			{
				if (!Actor) continue;

				if (Actor->IsA(ARoomBase::StaticClass()))
				{
					Room = Cast<ARoomBase>(Actor);
				}
				
				// 콘텐츠 필터링 (보물상자, 보스장식 등)
				if (Data.RoomType != ERoomType::Treasure && Actor->ActorHasTag(FName("TreasureContent"))) Actor->Destroy();
				if (Data.RoomType != ERoomType::Boss && Actor->ActorHasTag(FName("BossContent"))) Actor->Destroy();
			}
			
			if (Room)
			{
				Room->MyGridLocation = Data.GridLocation; 
				Room->SetRoomData(Data.DoorBitmask);
				RuntimeRoomMap.Add(Data.GridLocation, Room); 

				// [수정] 처리가 끝났음을 표시 (데이터를 삭제하지 않음!)
				Data.Depth = -999; 
			}

			// It.RemoveCurrent(); // <--- [삭제] 절대 지우면 안 됩니다!
		}
	}
}

void ADungeonGenerator::OnRep_DungeonLayout()
{
	SpawnDungeon();
}