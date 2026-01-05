
#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataAsset.h"
#include "UI/WidgetController/SWidgetController.h"
#include "ItemPickupWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemChangedSignature, const TArray<UItemDataAsset*>&, ItemList);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SPROJECT_API UItemPickupWidgetController : public USWidgetController
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryItemChangedSignature OnInventoryItemChanged;

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PickupItem(UItemDataAsset* ItemData);

	void SetLootItems(const TArray<UItemDataAsset*>& Items, AActor* InTargetBox);
	
protected:
	UPROPERTY()
	TArray<TObjectPtr<UItemDataAsset>> ItemList;

	UPROPERTY()
	TObjectPtr<AActor> TargetBox;
};
