

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

	ASPlayerState* PS = PlayerController->GetPlayerState<ASPlayerState>();
	if (PS && PS->Inventory)
	{
		// 1. 아이템 추가 요청 (이제 내부적으로 Server RPC를 탑니다!)
		PS->Inventory->AddItem(ItemData);
		
		// 2. 로컬 UI 리스트에서 제거 (내 화면에서 바로 지워지게 함)
		ItemList.RemoveSingle(ItemData);
		OnInventoryItemChanged.Broadcast(ItemList);
	}

	// 3. [상자 파괴 ⭐]
	if (ItemList.Num() == 0 && TargetBox)
	{
		// 서버에게 파괴를 요청해야 합니다.
		// 만약 TargetBox가 IInteractionInterface를 상속받았다면, 
		// 서버 RPC 함수를 하나 더 만들어서 호출해야 합니다.
		if (PlayerController->HasAuthority())
		{
			TargetBox->Destroy();
		}
		else
		{
			// 클라이언트는 서버에게 상자를 지워달라고 요청하는 별도의 무전이 필요합니다.
			// (이건 나중에 상자 클래스에 Server_Destroy 함수를 만들어서 해결하는게 정석입니다!)
		}
	}
}
void UItemPickupWidgetController::SetLootItems(const TArray<UItemDataAsset*>& Items, AActor* InTargetBox)
{
	ItemList = Items;
	TargetBox = InTargetBox;
	BroadcastInitialValues();
}

