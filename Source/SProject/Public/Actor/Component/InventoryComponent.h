
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actor/ItemBase.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdatedSignature, const TArray<UItemDataAsset*>&, InventoryItems);

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

	void UseItem(UItemDataAsset* ItemData);
protected:

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UItemDataAsset>> Inventory;

	
};
