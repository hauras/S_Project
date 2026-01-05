

#include "UI/WidgetController/ItemPickupWidgetController.h"

#include "Actor/Component/InventoryComponent.h"

void UItemPickupWidgetController::BroadcastInitialValues()
{
	OnInventoryItemChanged.Broadcast(ItemList);

}

void UItemPickupWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
}

void UItemPickupWidgetController::PickupItem(UItemDataAsset* ItemData)
{
	APawn* Player = PlayerController->GetPawn();
	UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();
	if (Inventory)
	{
		Inventory->AddItem(ItemData);

		ItemList.RemoveSingle(ItemData);

		OnInventoryItemChanged.Broadcast(ItemList);
	}
	if (ItemList.Num() == 0)
	{
		TargetBox->Destroy();
	}
}

void UItemPickupWidgetController::SetLootItems(const TArray<UItemDataAsset*>& Items, AActor* InTargetBox)
{
	ItemList = Items;
	TargetBox = InTargetBox;
	BroadcastInitialValues();
}

