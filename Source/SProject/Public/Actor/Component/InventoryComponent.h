#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataAsset.h"
#include "ActiveGameplayEffectHandle.h"
#include "InventoryComponent.generated.h"

// UI 갱신용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedSignature, const TArray<UItemDataAsset*>&, InventoryItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChangedSignature, EEquipmentSlot, Slot, UItemDataAsset*, ItemData);

USTRUCT(BlueprintType)
struct FEquippedItemInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquipmentSlot Slot = EEquipmentSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemDataAsset> ItemData = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	// 변수 복제를 위해 필요한 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<UItemDataAsset*>& GetInventoryList() const { return Inventory; }

	void AddItem(UItemDataAsset* ItemData);

	UFUNCTION(Server, Reliable)
	void Server_AddItem(UItemDataAsset* ItemData);
	
	void UseItem(UItemDataAsset* ItemData);

	// 아이템 사용 요청 (클라이언트가 호출 -> 서버에서 실행됨)
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_UseItem(UItemDataAsset* ItemData);

	// 인벤토리 데이터 로드 (서버에서만 실행)
	void LoadInventoryData(const TArray<TObjectPtr<UItemDataAsset>>& SavedInventory, const TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>>& SavedEquippedItems);

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdatedSignature OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnEquipmentChangedSignature OnEquipmentChanged;

	const TArray<FEquippedItemInfo>& GetEquippedItemsArray() const { return EquippedItemsArray; }

protected:
	// ReplicatedUsing: 서버에서 값이 바뀌면 클라이언트의 OnRep 함수가 자동 실행됨
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	TArray<TObjectPtr<UItemDataAsset>> Inventory;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedItems)
	TArray<FEquippedItemInfo> EquippedItemsArray;

	UFUNCTION() void OnRep_Inventory();
	UFUNCTION() void OnRep_EquippedItems();

	// GE 핸들은 서버에서만 관리하면 되므로 복제할 필요 없음
	UPROPERTY()
	TMap<EEquipmentSlot, FActiveGameplayEffectHandle> EquipmentEffectHandles;
};