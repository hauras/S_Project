
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "FrostFieldBase.generated.h"

class UGameplayEffect;
/**
 * 
 */
UCLASS()
class SPROJECT_API UFrostFieldBase : public USGameplayAbility
{
	GENERATED_BODY()
public:
	UFrostFieldBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	// 5스택 달성 시 적을 얼려버릴 스턴 GE
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Damage")
	TSubclassOf<UGameplayEffect> FreezeEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Damage")
	TSubclassOf<UGameplayEffect> ExplosionEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag FrostFieldTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<class AFrostField> FrostFieldClass;
private:

	UFUNCTION()
	void OnImpactEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();
};
