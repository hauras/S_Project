#include "Actor/Map/RoomBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

ARoomBase::ARoomBase()
{
	// 1. Root 설정 (액터의 중심점)
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 2. 바닥 메쉬 생성 및 부착
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(SceneRoot);

	// 3. 4방향 벽 메쉬 생성 및 부착
	Wall_North = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_North"));
	Wall_North->SetupAttachment(SceneRoot);

	Wall_South = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_South"));
	Wall_South->SetupAttachment(SceneRoot);

	Wall_West = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_West"));
	Wall_West->SetupAttachment(SceneRoot);

	Wall_East = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_East"));
	Wall_East->SetupAttachment(SceneRoot);
}

void ARoomBase::SetRoomConfiguration(int32 InBitmask)
{
	DoorBitmask = InBitmask;
	
	// 북쪽(Bit 1)이 켜져 있다면 = 북쪽에 문(통로)이 있어야 함 = 북쪽 벽을 없앤다
	if (DoorBitmask & 1)
	{
		Wall_North->SetVisibility(false);
		Wall_North->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 남쪽(Bit 2)이 켜져 있다면
	if (DoorBitmask & 2)
	{
		Wall_South->SetVisibility(false);
		Wall_South->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 서쪽(Bit 4)이 켜져 있다면
	if (DoorBitmask & 4)
	{
		Wall_West->SetVisibility(false);
		Wall_West->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 동쪽(Bit 8)이 켜져 있다면
	if (DoorBitmask & 8)
	{
		Wall_East->SetVisibility(false);
		Wall_East->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}