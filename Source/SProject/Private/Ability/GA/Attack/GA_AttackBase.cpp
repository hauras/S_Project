

#include "Ability/GA/Attack/GA_AttackBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"

UGA_AttackBase::UGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	TObjectPtr<UAnimMontage>* AnimMontage = AttackMontages.Find(CurrentCount);

	if (AnimMontage && *AnimMontage)
	{
		// ... 몽타주 재생 로직은 기존과 동일 ...
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, *AnimMontage);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_AttackBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_AttackBase::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_AttackBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// ★★★ 이 부분은 이제 필수입니다! ★★★
	// 데미지 이벤트를 기다리는 태스크를 활성화합니다.
	UAbilityTask_WaitGameplayEvent* WaitDamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetAttackTargetEventTag());
	WaitDamageEventTask->EventReceived.AddDynamic(this, &UGA_AttackBase::DoDamage);
	WaitDamageEventTask->ReadyForActivation();
}

FGameplayTag UGA_AttackBase::GetAttackTargetEventTag()
{
	return FGameplayTag::RequestGameplayTag("Event.Damage");
}

void UGA_AttackBase::OnMontageEnded()
{
	AdvanceCombo();

	GetWorld()->GetTimerManager().SetTimer(
	TimerHandle,
	this,
	&UGA_AttackBase::ResetCombo,
	AttackResetTime, 
	false
	);
	
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


void UGA_AttackBase::DoDamage(FGameplayEventData Data)
{
	AActor* TargetActor = const_cast<AActor*>(Data.Target.Get());
	if (!TargetActor)
	{
		return;
	}

	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DamageEffect is not set in %s"), *GetName());
		return;
	}

	FGameplayEffectContextHandle ContextHandle = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

	if (SpecHandle.IsValid())
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor);

		// --- [경고 해결을 위한 수정] ---
		// 함수의 반환값을 변수에 저장하여 "결과를 사용"하도록 합니다.
		// 지금 당장 이 변수를 사용하지 않더라도, 경고가 사라지고 더 안전한 코드가 됩니다.
		TArray<FActiveGameplayEffectHandle> AppliedEffectHandles = ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, TargetDataHandle);
		// --- [수정 끝] ---
	}
}