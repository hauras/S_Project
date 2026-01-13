#include "Actor/Component/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/ItemDataAsset.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Inventory);
	// ★ TMap 대신 TArray를 복제하도록 변경
	DOREPLIFETIME(UInventoryComponent, EquippedItemsArray);
}

void UInventoryComponent::AddItem(UItemDataAsset* ItemData)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_AddItem(ItemData);
		return;
	}

	// 서버라면 바로 실행
	Server_AddItem_Implementation(ItemData);
}

void UInventoryComponent::Server_AddItem_Implementation(UItemDataAsset* ItemData)
{
	if (ItemData)
	{
		Inventory.Add(ItemData);
		
		// 서버에서 방송 (서버 UI 갱신용)
		OnInventoryUpdated.Broadcast(Inventory);
		
		// 팁: Inventory가 ReplicatedUsing=OnRep_Inventory 설정이 되어있으므로
		// 서버에서 Add 하면 클라이언트의 OnRep_Inventory가 자동으로 실행되어
		// 클라이언트 UI도 갱신됩니다!
	}
}

void UInventoryComponent::UseItem(UItemDataAsset* ItemData)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_UseItem(ItemData);
	}
	else
	{
		// 2. 내가 서버라면? -> 바로 진짜 로직(_Implementation)을 실행합니다.
		Server_UseItem_Implementation(ItemData);
	}
}

void UInventoryComponent::Server_UseItem_Implementation(UItemDataAsset* ItemData)
{
	if (!ItemData || !Inventory.Contains(ItemData)) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC) return;

	if (ItemData->GetItemType() == EItemType::Equipment)
	{
		EEquipmentSlot Slot = ItemData->GetEquipmentSlot();

		// 1. [기존 장비 찾기 및 해제] 배열을 뒤져서 같은 부위가 있는지 확인합니다.
		int32 ExistingIndex = INDEX_NONE;
		for (int32 i = 0; i < EquippedItemsArray.Num(); ++i)
		{
			if (EquippedItemsArray[i].Slot == Slot)
			{
				ExistingIndex = i;
				break;
			}
		}

		if (ExistingIndex != INDEX_NONE)
		{
			// 기존 장비의 스탯 이펙트 제거 (영수증은 서버 전용 TMap에 보관 중)
			if (EquipmentEffectHandles.Contains(Slot))
			{
				ASC->RemoveActiveGameplayEffect(EquipmentEffectHandles[Slot]);
				EquipmentEffectHandles.Remove(Slot);
			}

			// 입고 있던 템을 다시 가방에 넣고 배열에서 삭제 (Swap)
			Inventory.Add(EquippedItemsArray[ExistingIndex].ItemData);
			EquippedItemsArray.RemoveAt(ExistingIndex);
		}

		// 2. [새 장비 효과 적용]
		if (ItemData->ItemEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemData->ItemEffectClass, 1.f, Context);
			FActiveGameplayEffectHandle NewHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
			// 새 영수증 보관 (서버 전용)
			EquipmentEffectHandles.Add(Slot, NewHandle);
		}

		// 3. [새 장비 장착] 배열에 새 정보 추가
		FEquippedItemInfo NewInfo;
		NewInfo.Slot = Slot;
		NewInfo.ItemData = ItemData;
		EquippedItemsArray.Add(NewInfo);

		// 서버 UI 방송 (장착 신호)
		OnEquipmentChanged.Broadcast(Slot, ItemData);
	}
	else // 소모품 로직
	{
		if (ItemData->ItemEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemData->ItemEffectClass, 1.f, Context);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	// [공통] 가방에서 삭제 및 UI 방송
	Inventory.RemoveSingle(ItemData);
	OnInventoryUpdated.Broadcast(Inventory);
}

void UInventoryComponent::OnRep_Inventory()
{
	OnInventoryUpdated.Broadcast(Inventory);
}

void UInventoryComponent::OnRep_EquippedItems()
{
	// 클라이언트가 서버로부터 장착 배열을 받으면 모든 슬롯 UI를 갱신합니다.
	for (const FEquippedItemInfo& Info : EquippedItemsArray)
	{
		OnEquipmentChanged.Broadcast(Info.Slot, Info.ItemData);
	}
}

void UInventoryComponent::LoadInventoryData(const TArray<TObjectPtr<UItemDataAsset>>& SavedInventory, const TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>>& SavedEquippedItems)
{
	if (!GetOwner()->HasAuthority()) return;

	Inventory = SavedInventory;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC) return;

	// 기존 핸들 및 배열 초기화
	for (auto& HandlePair : EquipmentEffectHandles)
	{
		ASC->RemoveActiveGameplayEffect(HandlePair.Value);
	}
	EquipmentEffectHandles.Empty();
	EquippedItemsArray.Empty();

	// 새 데이터 적용 (TMap으로 받은 세이브 데이터를 TArray로 변환하며 로드)
	for (auto& Pair : SavedEquippedItems)
	{
		if (Pair.Value && Pair.Value->ItemEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Pair.Value->ItemEffectClass, 1.f, Context);
			FActiveGameplayEffectHandle NewHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

			EquipmentEffectHandles.Add(Pair.Key, NewHandle);

			// 복제용 배열에 추가 ⭐
			FEquippedItemInfo LoadInfo;
			LoadInfo.Slot = Pair.Key;
			LoadInfo.ItemData = Pair.Value;
			EquippedItemsArray.Add(LoadInfo);

			// UI 방송
			OnEquipmentChanged.Broadcast(Pair.Key, Pair.Value);
		}
	}
	OnInventoryUpdated.Broadcast(Inventory);
}