
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LootTable.generated.h"

class UItemDataAsset;

USTRUCT(BlueprintType)
struct FLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UItemDataAsset> ItemData;

	UPROPERTY(EditAnywhere)
	float DropWeight = 1.0f;
};
/**
 * 
 */
UCLASS()
class SPROJECT_API ULootTable : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<FLootEntry> LootEntries;

	UItemDataAsset* RollItem();
};
