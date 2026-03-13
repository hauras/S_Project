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
	DOREPLIFETIME(UInventoryComponent, EquippedItemsArray);
}

void UInventoryComponent::AddItem(UItemDataAsset* ItemData)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_AddItem(ItemData);
		return;
	}

	Server_AddItem_Implementation(ItemData);
}

void UInventoryComponent::Server_AddItem_Implementation(UItemDataAsset* ItemData)
{
	if (ItemData)
	{
		Inventory.Add(ItemData);
		
		OnInventoryUpdated.Broadcast(Inventory);
		
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
			
			if (EquipmentEffectHandles.Contains(Slot))
			{
				ASC->RemoveActiveGameplayEffect(EquipmentEffectHandles[Slot]);
				EquipmentEffectHandles.Remove(Slot);
			}

			Inventory.Add(EquippedItemsArray[ExistingIndex].ItemData);
			EquippedItemsArray.RemoveAt(ExistingIndex);
		}

		// 2. [새 장비 효과 적용]
		if (ItemData->ItemEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemData->ItemEffectClass, 1.f, Context);
			FActiveGameplayEffectHandle NewHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
			EquipmentEffectHandles.Add(Slot, NewHandle);
		}

		FEquippedItemInfo NewInfo;
		NewInfo.Slot = Slot;
		NewInfo.ItemData = ItemData;
		EquippedItemsArray.Add(NewInfo);

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

	Inventory.RemoveSingle(ItemData);
	OnInventoryUpdated.Broadcast(Inventory);
}

void UInventoryComponent::OnRep_Inventory()
{
	OnInventoryUpdated.Broadcast(Inventory);
}

void UInventoryComponent::OnRep_EquippedItems()
{
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

	for (auto& HandlePair : EquipmentEffectHandles)
	{
		ASC->RemoveActiveGameplayEffect(HandlePair.Value);
	}
	EquipmentEffectHandles.Empty();
	EquippedItemsArray.Empty();

	for (auto& Pair : SavedEquippedItems)
	{
		if (Pair.Value && Pair.Value->ItemEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Pair.Value->ItemEffectClass, 1.f, Context);
			FActiveGameplayEffectHandle NewHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

			EquipmentEffectHandles.Add(Pair.Key, NewHandle);

			FEquippedItemInfo LoadInfo;
			LoadInfo.Slot = Pair.Key;
			LoadInfo.ItemData = Pair.Value;
			EquippedItemsArray.Add(LoadInfo);

			OnEquipmentChanged.Broadcast(Pair.Key, Pair.Value);
		}
	}
	OnInventoryUpdated.Broadcast(Inventory);
}