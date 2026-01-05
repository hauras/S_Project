
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actor/ItemBase.h"
#include "InventoryComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	void AddItem(UItemDataAsset* ItemData);

protected:

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UItemDataAsset>> Inventory;

	
};
