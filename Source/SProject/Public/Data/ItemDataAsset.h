
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataAsset.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Potion, // 소모형
	Scroll, // 강화/버프
	Quest,
	Equipment
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None,
	Weapon,
	Head,
	Chest,
	Legs,
	Feet
};


/**
 * 
 */
UCLASS()
class SPROJECT_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> ItemEffectClass;

	EItemType GetItemType() const { return ItemType; }

	EEquipmentSlot GetEquipmentSlot() const { return EquipmentSlot; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemDescription;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> ItemIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquipmentSlot EquipmentSlot;
};
