

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
	// 로그 1: 함수가 시작되었는가?
	UE_LOG(LogTemp, Warning, TEXT("C++: PickupItem 함수 호출됨!"));

	if (!PlayerController) {
		UE_LOG(LogTemp, Error, TEXT("C++: PlayerController가 Null입니다!"));
		return;
	}

	APawn* Player = PlayerController->GetPawn();
	if (!Player) {
		UE_LOG(LogTemp, Error, TEXT("C++: 조종 중인 캐릭터(Pawn)를 찾을 수 없습니다!"));
		return;
	}

	UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();
	if (Inventory)
	{
		Inventory->AddItem(ItemData);
		ItemList.RemoveSingle(ItemData);
		OnInventoryItemChanged.Broadcast(ItemList);
		
		UE_LOG(LogTemp, Warning, TEXT("C++: 가방에 아이템 추가 및 리스트 갱신 완료!"));
	}
	else
	{
		// ★ 만약 이 로그가 뜬다면 캐릭터에 인벤토리 컴포넌트가 안 붙은 겁니다!
		UE_LOG(LogTemp, Error, TEXT("C++: 캐릭터에서 InventoryComponent를 찾지 못했습니다!"));
	}

	if (ItemList.Num() == 0 && TargetBox) // TargetBox 유효성 체크 추가
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

