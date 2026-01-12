

#include "UI/WidgetController/ItemPickupWidgetController.h"

#include "Actor/Component/InventoryComponent.h"
#include "State/SPlayerState.h"

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
	if (!PlayerController) return;

	// ★ 수정: 캐릭터(Pawn)가 아닌 PlayerState에서 가방을 찾습니다.
	ASPlayerState* PS = PlayerController->GetPlayerState<ASPlayerState>();
	if (PS && PS->Inventory)
	{
		UInventoryComponent* Inventory = PS->Inventory;
		
		Inventory->AddItem(ItemData);
		ItemList.RemoveSingle(ItemData);
		OnInventoryItemChanged.Broadcast(ItemList);
		
		UE_LOG(LogTemp, Warning, TEXT("C++: (PlayerState) 가방에 아이템 추가 완료!"));
	}

	if (ItemList.Num() == 0 && TargetBox)
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

