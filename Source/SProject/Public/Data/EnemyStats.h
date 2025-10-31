
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyStats.generated.h"

class UGameplayEffect;
class UGameplayAbility;

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Melee,
	Ranged,
	Mage,
};

USTRUCT(BlueprintType)
struct FEnemyClassDefaultInfo
{
	GENERATED_BODY()
	
};
/**
 * 
 */
UCLASS()
class SPROJECT_API UEnemyStats : public UDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, Category = " Enemy Class Defaults")
	TMap<EEnemyType, FEnemyClassDefaultInfo> EnemyClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> MaxVitalAttributes;
	
	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> VitalAttributes;

	FEnemyClassDefaultInfo GetClassDefaultInfo(EEnemyType EnemyClass);

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;
};
