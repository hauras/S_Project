
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
	Quest, // 열쇠 등
};


/**
 * 
 */
UCLASS()
class SPROJECT_API UItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemDescription;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> ItemIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> ItemEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType;
};
