

#include "Actor/Component/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	
}

void UInventoryComponent::AddItem(UItemDataAsset* ItemData)
{
	Inventory.Add(ItemData);
}


