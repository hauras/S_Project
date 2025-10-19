

#include "Ability/GA/Attack/GA_AttackBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TimerManager.h"

UGA_AttackBase::UGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogTemp, Error, TEXT("DEBUG: 4. [Ability] ActivateAbility Called! Success!"));

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	TObjectPtr<UAnimMontage>* AnimMontage = AttackMontages.Find(CurrentCount);

	if (AnimMontage && *AnimMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			*AnimMontage
			);

		MontageTask->OnCompleted.AddDynamic(this, &UGA_AttackBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_AttackBase::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_AttackBase::OnMontageEnded);

		MontageTask->ReadyForActivation();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_AttackBase: No Montage found for Combo Count %d"), CurrentCount);
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UGA_AttackBase::OnMontageEnded()
{
	
		// 콤보를 다음 단계로 진행시킵니다.
	AdvanceCombo();

		// 다음 입력이 없으면 콤보를 리셋하기 위한 타이머를 시작합니다.
	GetWorld()->GetTimerManager().SetTimer(
	TimerHandle,
	this,
	&UGA_AttackBase::ResetCombo,
	AttackResetTime, // 블루프린트에서 설정 가능한 시간
	false
	);
	

	// 어빌리티의 현재 '실행'을 종료합니다.
	// 콤보 상태(CurrentComboCount)는 유지됩니다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_AttackBase::AdvanceCombo()
{
	if (AttackMontages.Contains(CurrentCount + 1))
	{
		CurrentCount++;
	}
	else
	{
		CurrentCount = 1;
	}
}

void UGA_AttackBase::ResetCombo()
{
	CurrentCount = 1;
}
