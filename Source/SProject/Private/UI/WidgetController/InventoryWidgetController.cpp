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
		UInventoryComponent* Inventory = PS->Inventory;
		if (Inventory)
		{
			InventoryItemsChangedDelegate.Broadcast(Inventory->GetInventoryList()); 
			for (auto& Pair : Inventory->EquippedItems)
			{
				EquipmentChangedDelegate.Broadcast(Pair.Key, Pair.Value);
			}
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
