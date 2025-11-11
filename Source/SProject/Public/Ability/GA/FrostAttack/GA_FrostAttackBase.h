
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "GA_FrostAttackBase.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API UGA_FrostAttackBase : public USGameplayAbility
{
	GENERATED_BODY()
public:

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAnimMontage> FrostAttackMontage;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hoarfrost")
	TSubclassOf<UGameplayEffect> DebuffEffectClass;*/ // 추후에 디버프 추가(슬로우등)

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float AttackRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UParticleSystem> FrostParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UParticleSystem> ImpactParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ClampMin = "1"))
	int32 ImpactParticle_Num = 8;
	
private:

	UFUNCTION()
	void OnMontageEnded();
};
