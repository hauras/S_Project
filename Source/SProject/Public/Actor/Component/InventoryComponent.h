
#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "Actor/ItemBase.h"
#include "Data/ItemDataAsset.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedSignature, const TArray<UItemDataAsset*>&,
                                            InventoryItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChangedSignature, EEquipmentSlot, Slot, UItemDataAsset*, ItemData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	const TArray<UItemDataAsset*>& GetInventoryList() const { return Inventory; }

	void AddItem(UItemDataAsset* ItemData);

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdatedSignature OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnEquipmentChangedSignature OnEquipmentChanged;

	UPROPERTY()
	TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>> EquippedItems;
	
	void UseItem(UItemDataAsset* ItemData);

	void LoadInventoryData(
			const TArray<TObjectPtr<UItemDataAsset>>& SavedInventory, 
			const TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>>& SavedEquippedItems
		);
protected:

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UItemDataAsset>> Inventory;

	UPROPERTY()
	TMap<EEquipmentSlot, FActiveGameplayEffectHandle> EquipmentEffectHandles;
};
