

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
		AActor* MyOwner = GetOwner();

		// 2. 라이브러리 도구를 써서 주인의 ASC를 가져옵니다.
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MyOwner);

		if (ASC)
		{
			UE_LOG(LogTemp, Warning, TEXT("주인의 ASC를 찾았습니다: %s"), *ASC->GetName());

			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ItemData->ItemEffectClass, 1.f, ContextHandle);
			if (SpecHandle.IsValid()) 
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				Inventory.RemoveSingle(ItemData);

				// [3] UI 새로고침 방송
				OnInventoryUpdated.Broadcast(Inventory);
			}

		}
	}
}


