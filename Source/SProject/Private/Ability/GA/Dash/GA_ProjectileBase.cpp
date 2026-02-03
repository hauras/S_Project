#include "Ability/GA/Dash/GA_ProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "SGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actor/Projectile.h"
#include "Character/CharacterBase.h"
#include "Interface/CombatInterface.h"
#include "Kismet/KismetMathLibrary.h" // ★ 회전 계산을 위해 추가 필요

void UGA_ProjectileBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Character) || !IsValid(ProjectileMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ProjectileMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_ProjectileBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_ProjectileBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTag); // 헤더의 변수 이름과 통일 (ProjectileTag -> FireEventTag)
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UGA_ProjectileBase::SpawnProjectile);
		EventTask->ReadyForActivation();
	}
}
void UGA_ProjectileBase::SpawnProjectile(FGameplayEventData Payload)
{
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if (!bIsServer) return;

    APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
    if (!AvatarPawn) return;

    ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarPawn);
    if (!CombatInterface || !ProjectileClass) return;

    const FVector SpawnLocation = CombatInterface->GetSocketLocation(ProjectileTag);
    
    FRotator LookRotation = FRotator::ZeroRotator;
    AController* OwnerController = AvatarPawn->GetController();

    if (OwnerController)
    {
        LookRotation = OwnerController->GetControlRotation();
    }

    if (LookRotation.IsNearlyZero() || !OwnerController)
    {
        LookRotation = AvatarPawn->GetActorRotation();
    }

    FVector TraceStart = AvatarPawn->GetPawnViewLocation(); 
    FVector TraceEnd = TraceStart + (LookRotation.Vector() * 5000.f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(AvatarPawn);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    FVector TargetLocation = bHit ? HitResult.Location : TraceEnd;

    // 4. 최종 발사 각도 계산 (총구 -> 목표 지점)
    FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);
	
    FTransform SpawnTransform(SpawnRotation, SpawnLocation);
    
    AProjectile* Projectile = GetWorld()->SpawnActorDeferred<AProjectile>(
        ProjectileClass,
        SpawnTransform,
        GetOwningActorFromActorInfo(),
        AvatarPawn,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (Projectile)
    {
        Projectile->DamageEffectClass = DamageEffectClass;
    	Projectile->TargetTag = MarkTag; 

        Projectile->FinishSpawning(SpawnTransform);
    }
}

void UGA_ProjectileBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}