#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "BeamAttackBase.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_WaitInputRelease;
class UGameplayEffect;

UCLASS()
class SPROJECT_API UBeamAttackBase : public USGameplayAbility
{
	GENERATED_BODY()

public:
	UBeamAttackBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	
	// --- 설정 데이터 ---
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Visual")
	TObjectPtr<UAnimMontage> BeamMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Tag")
	FGameplayTag FireEventTag; // 몽타주 노티파이 태그

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Tag")
	FGameplayTag SocketTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Tag")
	FGameplayTag BeamCueTag; // GameplayCue Tag (예: GameplayCue.Character.Beam)

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Stats")
	float BeamRange = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Stats")
	float TickRate = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Stats")
	TSubclassOf<UGameplayEffect> ManaDrainEffectClass;
	
	// --- 콜백 함수 ---
	UFUNCTION()
	void OnBeamEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnMontageEnded();

private:
	FTimerHandle BeamTimerHandle;
	bool bInputReleased = false; // 키를 뗐는지 추적하는 변수 추가

	// 매 틱마다 실행될 로직 (데미지 + 비주얼 업데이트)
	void TraceAndDamageTick();

	// GameplayCue 파라미터 업데이트용
	void UpdateBeamVisual(const FVector& ContactPoint);
};