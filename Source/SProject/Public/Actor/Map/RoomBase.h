#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomBase.generated.h"

// 컴포넌트들을 미리 알려주는 전방 선언 (컴파일 속도 향상)
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class SPROJECT_API ARoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoomBase();
	
	UFUNCTION(BlueprintCallable, Category = "Dungeon|Logic")
	void SetRoomConfiguration(int32 InBitmask);

protected:
	// --- 컴포넌트 구성 ---

	// 액터의 기준점이 될 Root
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// 바닥 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FloorMesh;

	// 4방향 벽 메쉬 (비트마스크 결과에 따라 제어됨)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Wall_North; // Bit 1

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Wall_South; // Bit 2

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Wall_West;  // Bit 4

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Wall_East;  // Bit 8

	// --- 데이터 ---

	// 현재 방의 연결 상태를 저장 (디버깅용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dungeon|Data")
	int32 DoorBitmask;

};