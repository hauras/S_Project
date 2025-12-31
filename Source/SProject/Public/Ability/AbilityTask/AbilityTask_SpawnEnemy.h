
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_SpawnEnemy.generated.h"
class AEnemyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnEnemyDelegate, const TArray<AEnemyCharacter*>&,SpawnedEnemies);

/**
 * 
 */
UCLASS()
class SPROJECT_API UAbilityTask_SpawnEnemy : public UAbilityTask
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "AbilityTasks", meta = (DisplayName = " Spawn Enemy"))
	static UAbilityTask_SpawnEnemy* SpawnEnemies(UGameplayAbility* OwningAbility, FGameplayTag EventTag, TSoftClassPtr<AEnemyCharacter> EnemyClassToSpawn, int32 NumToSpawn, const FVector& SpawnOrigin, float RandomSpawnRadius, const FRotator& SpawnRotator);

	UPROPERTY(BlueprintAssignable)
	FSpawnEnemyDelegate OnSpawnFinished;

	UPROPERTY(BlueprintAssignable)
	FSpawnEnemyDelegate DidNotSpawn;
protected:

	virtual void Activate() override;

	virtual void OnDestroy(bool bInOwnerFinished) override;
private:
	
	FGameplayTag CachedEventTag;
	TSoftClassPtr<AEnemyCharacter> CachedEnemyClassToSpawn;
	int32 CachedNumToSpawn;
	FVector CachedSpawnOrigin;
	float CachedRandomSpawnRadius;
	FRotator CachedSpawnRotator;

	FDelegateHandle DelegateHandle;
	void OnGameplayEventReceived(const FGameplayEventData* Payload);
};
