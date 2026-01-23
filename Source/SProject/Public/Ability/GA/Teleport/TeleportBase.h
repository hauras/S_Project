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
	FGameplayTag TeleportTag; // Event.Teleport (노티파이용)

	UPROPERTY(EditDefaultsOnly, Category = "Blink|Tag")
	FGameplayTag TeleportCueTag; // GameplayCue.Ability.Skill.Teleport (이펙트용)

	UPROPERTY(EditDefaultsOnly, Category = "Blink|Stats")
	float TeleportStrength = 5000.f; // 돌진 힘

	// [포폴 포인트] 잔상용 파티클 (에디터에서 P_PhaseLinkForm 선택)
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