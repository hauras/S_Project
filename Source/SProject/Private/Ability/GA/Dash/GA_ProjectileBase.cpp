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
    // 1. 서버 권한 확인
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if (!bIsServer) return;

    APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
    if (!AvatarPawn) return;

    ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarPawn);
    if (!CombatInterface || !ProjectileClass) return;

    // 2. 발사 시작 위치 (총구)
    const FVector SpawnLocation = CombatInterface->GetSocketLocation(FSGameplayTags::Get().Combat_Socket_Weapon);

    // -------------------------------------------------------------------------
    // ★ 조준 방향 계산 로직 (수정됨)
    // -------------------------------------------------------------------------
    
    FRotator LookRotation = FRotator::ZeroRotator;
    AController* OwnerController = AvatarPawn->GetController();

    // [중요] 컨트롤러가 있으면 컨트롤러 회전값(카메라 방향)을 우선 사용
    if (OwnerController)
    {
        LookRotation = OwnerController->GetControlRotation();
    }

    // [안전장치] 만약 컨트롤러 회전값이 (0,0,0)이거나 컨트롤러가 없다면 -> 캐릭터 몸이 보는 방향 사용
    // 이렇게 하면 절대 "한 방향(월드 X축)"으로만 나가는 버그가 사라집니다.
    if (LookRotation.IsNearlyZero() || !OwnerController)
    {
        LookRotation = AvatarPawn->GetActorRotation();
    }

    // 3. 트레이스 (레이저 쏘기)
    // 안전장치를 거친 LookRotation을 기준으로 목표 지점을 찾습니다.
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
        Projectile->FinishSpawning(SpawnTransform);
    }
}

void UGA_ProjectileBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}