#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonGenerator.generated.h"

class UBoxComponent;
// [추가] 레벨 스트리밍 클래스 전방 선언
class ULevelStreamingDynamic;


UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Start,
	Normal,
	Boss,
	Treasure
};

USTRUCT(BlueprintType)
struct FRoomData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FIntPoint GridLocation;

	UPROPERTY(BlueprintReadOnly)
	int32 DoorBitmask = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Depth = 0;

	UPROPERTY(BlueprintReadOnly)
	ERoomType RoomType = ERoomType::Normal;
};


UCLASS()
class SPROJECT_API ADungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	ADungeonGenerator();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	TMap<FIntPoint, class ARoomBase*> RuntimeRoomMap;

	// 특정 방이 클리어되었을 때 주변 방들에게 문을 열라고 시키는 함수
	void NotifyNeighborDoors(FIntPoint ClearedCoords, int32 Bitmask);

	FORCEINLINE const TArray<FRoomData>& GetDungeonLayout() const { return DungeonLayout; }

	UFUNCTION(BlueprintPure, Category = "Dungeon")
	FIntPoint GetGridCoordsFromWorldLocation(FVector WorldLocation) const;
	
protected:
	virtual void BeginPlay() override;
	
    UPROPERTY(ReplicatedUsing = OnRep_DungeonLayout, BlueprintReadOnly, Category = "Dungeon")
	TArray<FRoomData> DungeonLayout;

	UPROPERTY(EditAnywhere, Category = "Dungeon Settings")
	int32 MaxRooms = 16;
	
	UPROPERTY(EditAnywhere, Category = "Dungeon Settings")
	TSoftObjectPtr<UWorld> RoomLevel;
	
	UPROPERTY(EditAnywhere, Category = "Dungeon Settings")
	float RoomSize = 9000.f;

	UPROPERTY()
	TMap<ULevelStreamingDynamic*, FRoomData> LevelDataMap;

	UPROPERTY()
	TMap<FIntPoint, FRoomData> RoomMap;
	
	UPROPERTY(EditAnywhere, Category = "Dungeon Settings")
	float TreasureRoomRatio = 0.3f;

	virtual void Tick(float DeltaTime) override;

	// 지난 프레임에 플레이어가 있던 방 좌표 (중복 계산 방지)
	FIntPoint LastPlayerGridPos = FIntPoint(-999, -999);

	// 최적화 실행 함수
	void UpdateRoomVisibility(FIntPoint CurrentGridPos);

	void InitializeRoom(ULevelStreamingDynamic* InStreamingLevel, FRoomData& Data);
	
	void AssignSpecialRooms();
	// [추가] 레벨 로딩이 완료(Shown)되었을 때 엔진이 자동으로 호출해줄 함수
	UFUNCTION()
	void OnRoomLevelShown();

	UFUNCTION()
	void OnRep_DungeonLayout();

	void GenerateDungeon();

	void SpawnDungeon();
};