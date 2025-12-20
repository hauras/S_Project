
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "GA_EnemyProjectileBase.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class SPROJECT_API UGA_EnemyProjectileBase : public USGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ProjectileTag;

private:
	UFUNCTION()
	void FireProjectile(FGameplayEventData Payload);
	UFUNCTION()
	void OnMontageEnded();
};
