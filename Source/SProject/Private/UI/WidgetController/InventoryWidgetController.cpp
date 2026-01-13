#include "UI/WidgetController/InventoryWidgetController.h"
#include "Actor/Component/InventoryComponent.h"
#include "State/SPlayerState.h"

// 1. 초기값 방송 (창이 처음 켜질 때 실행됨)
void UInventoryWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	ASPlayerState* PS = PlayerController->GetPlayerState<ASPlayerState>();
	if (PS && PS->Inventory)
	{
		// 1. 가방 리스트 방송 (기존과 동일)
		InventoryItemsChangedDelegate.Broadcast(PS->Inventory->GetInventoryList()); 

		// 2. [수정] 장착 아이템 방송
		// 이제 TMap이 아니라 TArray를 순회하며 하나씩 쏴줍니다.
		// (InventoryComponent에 'EquippedItemsArray'가 public이거나 Getter가 있어야 합니다)
		for (const FEquippedItemInfo& Info : PS->Inventory->GetEquippedItemsArray())
		{
			// 구조체 안에 들어있는 Slot(부위)과 ItemData(아이콘)를 분리해서 방송!
			EquipmentChangedDelegate.Broadcast(Info.Slot, Info.ItemData);
		}
	}
}

void UInventoryWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();

	ASPlayerState* PS = PlayerController->GetPlayerState<ASPlayerState>();
	if (PS && PS->Inventory)
	{
		PS->Inventory->OnInventoryUpdated.AddDynamic(this, &UInventoryWidgetController::OnInventoryUpdatedCallback);
		PS->Inventory->OnEquipmentChanged.AddDynamic(this, &UInventoryWidgetController::OnEquipmentChangedCallback);
	}
}

// 2. 아이템 사용 (위젯에서 아이콘을 클릭하면 호출됨)
void UInventoryWidgetController::UseItem(UItemDataAsset* ItemAsset)
{
	ASPlayerState* PS = PlayerController->GetPlayerState<ASPlayerState>();
	if (PS && PS->Inventory && ItemAsset)
	{
		PS->Inventory->UseItem(ItemAsset);
	}
}

void UInventoryWidgetController::OnInventoryUpdatedCallback(const TArray<UItemDataAsset*>& Items)
{
	InventoryItemsChangedDelegate.Broadcast(Items);
}

void UInventoryWidgetController::OnEquipmentChangedCallback(EEquipmentSlot Slot, UItemDataAsset* ItemData)
{
	EquipmentChangedDelegate.Broadcast(Slot, ItemData);
}
