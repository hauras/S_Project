#include "UI/WidgetController/InventoryWidgetController.h"
#include "Actor/Component/InventoryComponent.h"

// 1. 초기값 방송 (창이 처음 켜질 때 실행됨)
void UInventoryWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	// 조종 중인 캐릭터를 가져옵니다.
	APawn* ControlledPawn = PlayerController->GetPawn();
	if (ControlledPawn)
	{
		// 가방 컴포넌트를 찾습니다.
		UInventoryComponent* Inventory = ControlledPawn->FindComponentByClass<UInventoryComponent>();
		if (Inventory)
		{
			// [핵심] 가방이 지금 가지고 있는 진짜 아이템 리스트를 
			// 위젯(UI)에게 "자, 이게 지금 네 가방 내용물이야!"라고 처음으로 알려줍니다.
			// (InventoryComponent의 배열 이름이 'Inventory'라고 가정합니다)
			InventoryItemsChangedDelegate.Broadcast(Inventory->GetInventoryList()); 
		}
	}
}

void UInventoryWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();

	APawn* ControlledPawn = PlayerController->GetPawn();
	if (ControlledPawn)
	{
		UInventoryComponent* Inventory = ControlledPawn->FindComponentByClass<UInventoryComponent>();
		if (Inventory)
		{
			// 가방 데이터가 변할 때마다(추가/삭제) 내 귀(Callback 함수)로 들려달라고 예약합니다.
			Inventory->OnInventoryUpdated.AddDynamic(this, &UInventoryWidgetController::OnInventoryUpdatedCallback);
		}
	}
}

// 2. 아이템 사용 (위젯에서 아이콘을 클릭하면 호출됨)
void UInventoryWidgetController::UseItem(UItemDataAsset* ItemAsset)
{
	APawn* ControlledPawn = PlayerController->GetPawn();
	if (ControlledPawn)
	{
		UInventoryComponent* Inventory = ControlledPawn->FindComponentByClass<UInventoryComponent>();
		if (Inventory && ItemAsset)
		{
			// [핵심] 가방 컴포넌트에게 "이 아이템 진짜로 사용해!"라고 명령을 전달합니다.
			// 그러면 우리가 어제 짰던 [효과 적용 -> 소모 -> 다시 방송] 로직이 실행됩니다.
			Inventory->UseItem(ItemAsset);
		}
	}
}

void UInventoryWidgetController::OnInventoryUpdatedCallback(const TArray<UItemDataAsset*>& Items)
{
	// 가방에서 온 소식을 그대로 UI(델리게이트)에게 전달합니다.
	InventoryItemsChangedDelegate.Broadcast(Items);
}