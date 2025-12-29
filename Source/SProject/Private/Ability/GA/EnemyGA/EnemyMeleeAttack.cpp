#include "Ability/GA/EnemyGA/EnemyMeleeAttack.h"
#include "Character/CharacterBase.h"
#include "Interface/EnemyInterface.h" // ★★★ 인터페이스 헤더가 반드시 필요합니다! ★★★
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h" // SphereOverlap 등을 위해
#include "AbilitySystemComponent.h"
#include "SGameplayTags.h" // GameplayTags를 사용하기 위해
#include "Ability/SAbilityFunctionLibrary.h"

void UEnemyMeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* CombatTarget = nullptr;
	if (AvatarActor->Implements<UEnemyInterface>())
	{
		CombatTarget = IEnemyInterface::Execute_GetCombatTarget(AvatarActor);
	}

	if (CombatTarget == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}


	if (AvatarActor->Implements<UCombatInterface>())
	{
		ICombatInterface::Execute_UpdateFacingTarget(AvatarActor, CombatTarget->GetActorLocation());
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UEnemyMeleeAttack::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UEnemyMeleeAttack::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DamageEventTag, nullptr, false, false);
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UEnemyMeleeAttack::HandleDamageEvent);
		EventTask->ReadyForActivation();
	}

	const float UpdateFrequency = 1.f / 60.f;
	GetWorld()->GetTimerManager().SetTimer(
		WarpTargetTimer,
		this,
		&UEnemyMeleeAttack::UpdateWarpTarget,
		UpdateFrequency,
		true
	);
	
}
void UEnemyMeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(WarpTargetTimer);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UEnemyMeleeAttack::HandleDamageEvent(FGameplayEventData Payload)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	// 데미지 판정 로직
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface == nullptr) return;

	const FVector SocketLocation = CombatInterface->GetSocketLocation(FSGameplayTags::Get().Combat_Socket_Weapon);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetAvatarActorFromActorInfo());

	TArray<AActor*> OverlappingActors;

	USAbilityFunctionLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		IgnoreActors,
		50.f,
		SocketLocation
		);

	if (OverlappingActors.Num() > 0 && DamageEffectClass)
	{
		FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());

		for (AActor* TargetActor : OverlappingActors)
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			}
		}
	}
}

void UEnemyMeleeAttack::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UEnemyMeleeAttack::UpdateWarpTarget()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr) return;
	
	IEnemyInterface* EnemyInterface = Cast<IEnemyInterface>(AvatarActor);
	if (EnemyInterface == nullptr) return;

	// 최신 CombatTarget 정보를 가져옵니다.
	AActor* CombatTarget = IEnemyInterface::Execute_GetCombatTarget(AvatarActor);
	if (CombatTarget == nullptr) return;

	ICombatInterface* CombatInterface_Owner = Cast<ICombatInterface>(AvatarActor);
	if (CombatInterface_Owner == nullptr) return;

	// 매 프레임마다 모션 워핑의 목표 지점을 최신 타겟 위치로 업데이트합니다.
	CombatInterface_Owner->Execute_UpdateFacingTarget(AvatarActor, CombatTarget->GetActorLocation());
}

