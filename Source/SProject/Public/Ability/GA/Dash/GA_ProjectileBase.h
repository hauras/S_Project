
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "GA_ProjectileBase.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class SPROJECT_API UGA_ProjectileBase : public USGameplayAbility
{
	GENERATED_BODY()
public:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAnimMontage> ProjectileMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag ProjectileTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag FireEventTag;
private:
	
	UFUNCTION()
	void SpawnProjectile(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageEnded();
};
