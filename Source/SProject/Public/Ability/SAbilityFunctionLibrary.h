
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/EnemyStats.h"
#include "SAbilityFunctionLibrary.generated.h"

class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class SPROJECT_API USAbilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Ability")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, EEnemyType EnemyType, float Level, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category="AbilitySystemLibrary|CharacterClassDefaults" )
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "AbilitySystemLibrary|CharacterClassDefaults")
	static UEnemyStats* GetMonsterClassInfo(const UObject* WorldContextObject);

	
};
