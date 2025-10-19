
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/SGameplayAbility.h"
#include "GA_AttackBase.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API UGA_AttackBase : public USGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_AttackBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TMap<int32, TObjectPtr<UAnimMontage>> AttackMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackResetTime = 1.f;

	UFUNCTION()
	void OnMontageEnded();

	// 콤보를 다음 단계로 진행시키는 함수
	void AdvanceCombo();

	// 콤보 카운트를 초기화하는 함수
	UFUNCTION() // 타이머가 호출할 수 있도록 UFUNCTION() 추가
	void ResetCombo();
private:
	
	int32 CurrentCount = 1;
	FTimerHandle TimerHandle;
	
};
