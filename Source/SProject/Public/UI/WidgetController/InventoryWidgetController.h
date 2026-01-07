
#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataAsset.h"
#include "UI/WidgetController/SWidgetController.h"
#include "InventoryWidgetController.generated.h"

class UItemDataAsset;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedControllerSignature, const TArray<UItemDataAsset*>&, InventoryItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChangedControllerSignature, EEquipmentSlot, Slot, UItemDataAsset*, ItemData);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SPROJECT_API UInventoryWidgetController : public USWidgetController
{
	GENERATED_BODY()
public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(UItemDataAsset* ItemAsset);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdatedControllerSignature InventoryItemsChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEquipmentChangedControllerSignature EquipmentChangedDelegate;
	
	UFUNCTION()
	void OnInventoryUpdatedCallback(const TArray<UItemDataAsset*>& Items);

private:

	UFUNCTION()
	void OnEquipmentChangedCallback(EEquipmentSlot Slot, UItemDataAsset* ItemData);};
