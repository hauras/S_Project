
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "AoeStunBase.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API UAoeStunBase : public USGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float FlareRadius = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAnimMontage> FlareMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> FlareEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag FlareTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UParticleSystem> FlareParticle;
	

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	
	UFUNCTION()
	void OnFlareEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();
};
