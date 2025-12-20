

#include "Ability/GA/EnemyGA/Projectile/GA_EnemyProjectileBase.h"

#include "SGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Interface/CombatInterface.h"
#include "Interface/EnemyInterface.h"
#include "Actor/Projectile.h"
void UGA_EnemyProjectileBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FireMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyProjectileBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyProjectileBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ProjectileTag);
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UGA_EnemyProjectileBase::FireProjectile);
		EventTask->ReadyForActivation();
	}
}

void UGA_EnemyProjectileBase::FireProjectile(FGameplayEventData Payload)
{
	// 서버에서만 실행하도록 보장합니다.
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) 
	{
		return;
	}

	// 1. IEnemyInterface를 통해 현재 공격 대상(CombatTarget)을 가져옵니다.
	IEnemyInterface* EnemyInterface = Cast<IEnemyInterface>(GetAvatarActorFromActorInfo());
	if (EnemyInterface == nullptr) 
	{
		return;
	}
	AActor* CombatTarget = IEnemyInterface::Execute_GetCombatTarget(GetAvatarActorFromActorInfo());
	if (CombatTarget == nullptr) 
	{
		return;
	}

	// 2. ICombatInterface를 통해 발사 '위치'를 가져옵니다.
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface == nullptr) 
	{
		return;
	}
	const FVector SpawnLocation = CombatInterface->GetSocketLocation(FSGameplayTags::Get().Combat_Socket_Weapon);

	// 3. '발사 위치'에서 '타겟 위치'를 바라보는 '방향'을 계산합니다.
	const FRotator SpawnRotation = (CombatTarget->GetActorLocation() - SpawnLocation).Rotation();

	// 4. 위치와 방향 정보를 FTransform으로 합칩니다.
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	// 5. 계산된 위치와 방향으로 '총알' 액터를 생성합니다.
	AProjectile* Projectile = GetWorld()->SpawnActorDeferred<AProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetAvatarActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	// 6. 총알에 데미지 정보를 전달하고 최종 스폰합니다.
	if (Projectile)
	{
		Projectile->DamageEffectClass = DamageEffectClass;
		Projectile->FinishSpawning(SpawnTransform);
	}
}

void UGA_EnemyProjectileBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

}
