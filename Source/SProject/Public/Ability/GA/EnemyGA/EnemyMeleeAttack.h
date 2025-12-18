
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "EnemyMeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API UEnemyMeleeAttack : public USGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DamageEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "Timing")
	float TurnDelay = 0.15f;
private:

	UFUNCTION()
	void HandleDamageEvent(FGameplayEventData Payload);
	UFUNCTION()
	void OnMontageEnded();

	void UpdateWarpTarget();
	FTimerHandle WarpTargetTimer;
};
