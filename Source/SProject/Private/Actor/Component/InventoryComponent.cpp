

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
        
				// 영구 지속(Infinite) 이펙트를 적용하고 영수증(Handle)을 받습니다.
				FActiveGameplayEffectHandle NewHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        
				// 나중에 무기 벗을 때 쓰려고 영수증을 저장해둡니다.
				EquipmentEffectHandles.Add(Slot, NewHandle);
			}
			
			if (EquippedItems.Contains(Slot))
			{
				Inventory.Add(EquippedItems[Slot]); // 옛날 템 가방행
			}

			// 새 템 장착
			EquippedItems.Add(Slot, ItemData);
            
			// 장착 UI에게 알림
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


