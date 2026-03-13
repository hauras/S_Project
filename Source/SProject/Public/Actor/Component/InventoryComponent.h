#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataAsset.h"
#include "ActiveGameplayEffectHandle.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedSignature, const TArray<UItemDataAsset*>&, InventoryItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChangedSignature, EEquipmentSlot, Slot, UItemDataAsset*, ItemData);

USTRUCT(BlueprintType)
struct FEquippedItemInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquipmentSlot Slot = EEquipmentSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UItemDataAsset> ItemData = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<UItemDataAsset*>& GetInventoryList() const { return Inventory; }

	void AddItem(UItemDataAsset* ItemData);

	UFUNCTION(Server, Reliable)
	void Server_AddItem(UItemDataAsset* ItemData);
	
	void UseItem(UItemDataAsset* ItemData);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_UseItem(UItemDataAsset* ItemData);

	void LoadInventoryData(const TArray<TObjectPtr<UItemDataAsset>>& SavedInventory, const TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>>& SavedEquippedItems);

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnInventoryUpdatedSignature OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnEquipmentChangedSignature OnEquipmentChanged;

	const TArray<FEquippedItemInfo>& GetEquippedItemsArray() const { return EquippedItemsArray; }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	TArray<TObjectPtr<UItemDataAsset>> Inventory;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedItems)
	TArray<FEquippedItemInfo> EquippedItemsArray;

	UFUNCTION() void OnRep_Inventory();
	UFUNCTION() void OnRep_EquippedItems();

	UPROPERTY()
	TMap<EEquipmentSlot, FActiveGameplayEffectHandle> EquipmentEffectHandles;
};