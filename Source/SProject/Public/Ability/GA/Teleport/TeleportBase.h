#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "TeleportBase.generated.h"

class UNiagaraSystem;
class UAnimMontage;

UCLASS()
class SPROJECT_API UTeleportBase : public USGameplayAbility
{
	GENERATED_BODY()

public:
	UTeleportBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// --- 설정 데이터 ---
	UPROPERTY(EditDefaultsOnly, Category = "Blink|Visual")
	TObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Blink|Tag")
	FGameplayTag TeleportTag; 

	UPROPERTY(EditDefaultsOnly, Category = "Blink|Tag")
	FGameplayTag TeleportCueTag; 

	UPROPERTY(EditDefaultsOnly, Category = "Blink|Stats")
	float TeleportStrength = 5000.f; 

	UPROPERTY(EditDefaultsOnly, Category = "Blink|Visual")
	TObjectPtr<UNiagaraSystem> TrailParticle;

	// --- 콜백 함수 ---
	UFUNCTION()
	void OnTeleportEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEnded();

	// 잔상 스폰 함수
	void SpawnTrail();

private:
	FTimerHandle TrailTimerHandle;
};