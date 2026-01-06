
#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/SWidgetController.h"
#include "InventoryWidgetController.generated.h"

class UItemDataAsset;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedControllerSignature, const TArray<UItemDataAsset*>&, InventoryItems);

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

	UFUNCTION()
	void OnInventoryUpdatedCallback(const TArray<UItemDataAsset*>& Items);
};
