
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "BlackHoleBase.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API UBlackHoleBase : public USGameplayAbility
{
	GENERATED_BODY()
public:
	UBlackHoleBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float SpawnDistance = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag BlackHoleTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<AActor> BlackHoleClass;

	
	UFUNCTION()
	void OnMontageEnded();

	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);
};
