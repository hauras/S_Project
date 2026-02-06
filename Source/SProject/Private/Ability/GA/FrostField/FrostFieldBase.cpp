

#include "Ability/GA/FrostField/FrostFieldBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actor/FrostField.h"
#include "Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
UFrostFieldBase::UFrostFieldBase()
{
	
}

void UFrostFieldBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Player) || !IsValid(AnimMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. [순서 변경] 이벤트를 기다리는 태스크를 먼저 활성화 (귀를 먼저 연다)
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FrostFieldTag);
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UFrostFieldBase::OnImpactEventReceived);
		EventTask->ReadyForActivation();
	}

	// 2. 몽타주 재생 태스크 활성화
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AnimMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UFrostFieldBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UFrostFieldBase::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UFrostFieldBase::OnMontageEnded); // Cancelled도 추가하는 게 안전합니다.
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UFrostFieldBase::OnImpactEventReceived(FGameplayEventData Payload)
{
	// 1. [권한 관리] 서버에서만 액터 스폰을 수행하여 데이터 무결성 보장
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	// --- [단계 1: 정밀 지면 탐색 (Line Trace)] ---
	// 캐릭터 발 위치보다 살짝 위(+100)에서 바닥 방향으로 레이저를 발사
	FVector Start = Avatar->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	FVector End = Start - FVector(0.f, 0.f, 500.f);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(Avatar);

	// 월드 지형(WorldStatic) 채널을 탐색
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, TraceParams);

	/** 
	 * [기술적 디테일] ImpactPoint 활용
	 * Location 대신 ImpactPoint를 사용하여 경사면에서도 장판이 지면에 정확히 밀착되도록 구현 
	 */
	FVector SpawnLocation = bHit ? HitResult.ImpactPoint : Avatar->GetActorLocation() - FVector(0.f, 0.f, 90.f);

	// --- [단계 2: 액터 소환 및 데이터 주입 (Deferred Spawn)] ---
	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	
	// SpawnActorDeferred를 사용해 객체 생성 시점과 배치 시점을 분리
	AFrostField* FrostFieldActor = GetWorld()->SpawnActorDeferred<AFrostField>(
		FrostFieldClass, 
		SpawnTransform, 
		Avatar, 
		Cast<APawn>(Avatar), 
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (FrostFieldActor)
	{
		FrostFieldActor->FreezeEffectClass = FreezeEffectClass;
		FrostFieldActor->ExplosionEffectClass = ExplosionEffectClass; // [추가]

		FrostFieldActor->FinishSpawning(SpawnTransform);
	}
	
}

void UFrostFieldBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

}
