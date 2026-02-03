#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "GA_FrostAttackBase.generated.h"

UCLASS()
class SPROJECT_API UGA_FrostAttackBase : public USGameplayAbility
{
	GENERATED_BODY()

public:

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<UAnimMontage> FrostAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float AttackRadius = 500.f;

	// --- [GameplayCue 시스템으로 변경] ---
	// 기존 UParticleSystem* 대신 태그를 사용하여 네트워크 복제를 자동화합니다.

	UPROPERTY(EditDefaultsOnly, Category = "Visual|GameplayCue")
	FGameplayTag FrostCueTag;  // 중심부 폭발용 태그 (예: GameplayCue.Frost.Center)

	UPROPERTY(EditDefaultsOnly, Category = "Visual|GameplayCue")
	FGameplayTag ImpactCueTag; // 주변 파편용 태그 (예: GameplayCue.Frost.Impact)

	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (ClampMin = "1"))
	int32 ImpactParticle_Num = 8;
	
private:
	UFUNCTION()
	void OnMontageEnded();
};