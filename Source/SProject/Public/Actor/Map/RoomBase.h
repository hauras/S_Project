#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actor/Map/DungeonGenerator.h"
#include "RoomBase.generated.h"

class UBoxComponent;

UCLASS()
class SPROJECT_API ARoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoomBase();
	
	void SetRoomData(int32 InBitmask) { MyDoorBitmask = InBitmask; }

	void OpenDoors();
	void EnemyDied();
	void CacheInternalActors();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Data")
	FIntPoint MyGridLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Logic")
	TObjectPtr<ADungeonGenerator> MyGenerator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Data")
	ERoomType MyRoomType;

	void SetRoomType(ERoomType InType) { MyRoomType = InType; }
	
protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Logic")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Data")
	int32 MyDoorBitmask = 0;

	// 3. 캐릭터가 들어왔을 때 실행될 함수
	UFUNCTION()
	void OnPlayerEntered(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Room")
	TSubclassOf<APawn> MeleeMonster;

	UPROPERTY(EditAnywhere, Category = "Room")
	TSubclassOf<APawn> RangedMonster;

	UPROPERTY(EditAnywhere, Category = "Room")
	TSubclassOf<APawn> BossMonster;
	
	int32 MonsterCount = 0;

	void SpawnEnemy();

	void CloseAllGates();

	UPROPERTY()
	TArray<AActor*> NorthGates;
	UPROPERTY()
	TArray<AActor*> EastGates;
	UPROPERTY()
	TArray<AActor*> SouthGates;
	UPROPERTY()
	TArray<AActor*> WestGates;

	UPROPERTY()
	TArray<AActor*> MeleePoints;
	UPROPERTY()
	TArray<AActor*> RangedPoints;
	UPROPERTY()
	TArray<AActor*> BossPoints;
	
};