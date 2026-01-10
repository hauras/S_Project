

#include "Actor/Component/InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/PlayerCharacter.h"
#include "Data/ItemDataAsset.h"

UInventoryComponent::UInventoryComponent()
{
	
}

void UInventoryComponent::AddItem(UItemDataAsset* ItemData)
{
	Inventory.Add(ItemData);
	OnInventoryUpdated.Broadcast(Inventory);
}

void UInventoryComponent::UseItem(UItemDataAsset* ItemData)
{
	if (ItemData && Inventory.Contains(ItemData))
	{
		// [1] 장비 아이템인 경우
		if (ItemData->GetItemType() == EItemType::Equipment)
		{
			EEquipmentSlot Slot = ItemData->GetEquipmentSlot();
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

			if (EquipmentEffectHandles.Contains(Slot))
			{
				ASC->RemoveActiveGameplayEffect(EquipmentEffectHandles[Slot]);
			}
			
			if (ItemData->ItemEffectClass && ASC)
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemData->ItemEffectClass, 1.f, Context);
        
				FActiveGameplayEffectHandle NewHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        
				EquipmentEffectHandles.Add(Slot, NewHandle);
			}
			
			if (EquippedItems.Contains(Slot))
			{
				Inventory.Add(EquippedItems[Slot]); 
			}

			// 새 템 장착
			EquippedItems.Add(Slot, ItemData);
            
			OnEquipmentChanged.Broadcast(Slot, ItemData);
		}
		// [2] 소모품(포션 등)인 경우
		else
		{
			AActor* MyOwner = GetOwner();
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MyOwner);

			if (ASC)
			{
				FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ItemData->ItemEffectClass, 1.f, ContextHandle);
				if (SpecHandle.IsValid()) 
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
		Inventory.RemoveSingle(ItemData);
		OnInventoryUpdated.Broadcast(Inventory);
	}
}

void UInventoryComponent::LoadInventoryData(const TArray<TObjectPtr<UItemDataAsset>>& SavedInventory,
	const TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>>& SavedEquippedItems)
{
	Inventory = SavedInventory;
	OnInventoryUpdated.Broadcast(Inventory);

	// 2. [핵심 ⭐] 장착 데이터 복구 및 '재장착' 프로세스
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

	for (auto& Pair : SavedEquippedItems)
	{
		EEquipmentSlot Slot = Pair.Key;
		UItemDataAsset* Item = Pair.Value;

		if (Item && ASC)
		{
			// [A] 실제로 이펙트를 다시 걸어줍니다. (스탯 보너스 부활)
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Item->ItemEffectClass, 1.f, Context);
			FActiveGameplayEffectHandle NewHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

			// [B] 새로운 맵에서의 영수증을 저장합니다.
			EquipmentEffectHandles.Add(Slot, NewHandle);
			EquippedItems.Add(Slot, Item);

			// [C] UI 슬롯에게도 알려줍니다.
			OnEquipmentChanged.Broadcast(Slot, Item);
		}
	}
}

