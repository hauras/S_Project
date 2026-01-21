#include "Ability/GA/BeamAttack/BeamAttackBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Interface/CombatInterface.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"

UBeamAttackBase::UBeamAttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UBeamAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	bInputReleased = false;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!BeamMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 1. 입력 해제 대기
	UAbilityTask_WaitInputRelease* InputWaitTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (InputWaitTask)
	{
		InputWaitTask->OnRelease.AddDynamic(this, &UBeamAttackBase::OnInputReleased);
		InputWaitTask->ReadyForActivation();
	}

	// 2. 몽타주 재생 (Start 섹션부터)
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("Start"), BeamMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UBeamAttackBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UBeamAttackBase::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UBeamAttackBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}

	// 3. 애니메이션 노티파이(발사 시작) 대기
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FireEventTag);
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UBeamAttackBase::OnBeamEventReceived);
		EventTask->ReadyForActivation();
	}
}

void UBeamAttackBase::OnInputReleased(float TimeHeld)
{
	bInputReleased = true;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UBeamAttackBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UBeamAttackBase::OnBeamEventReceived(FGameplayEventData Payload)
{
	// 로그 추가: 이벤트가 실제로 들어오는지 확인
	UE_LOG(LogTemp, Warning, TEXT("GA_Beam: Event received!"));

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		// ★ 이게 없으면 BeamBase의 Tick이 통째로 안 돌아!
		CueParams.EffectCauser = GetAvatarActorFromActorInfo(); 
		CueParams.Location = GetAvatarActorFromActorInfo()->GetActorLocation(); // 임시 위치
        
		ASC->AddGameplayCue(BeamCueTag, CueParams);
		UE_LOG(LogTemp, Warning, TEXT("GA_Beam: AddGameplayCue Called with Tag: %s"), *BeamCueTag.ToString());
	}

	GetWorld()->GetTimerManager().SetTimer(BeamTimerHandle, this, &UBeamAttackBase::TraceAndDamageTick, TickRate, true);
}

void UBeamAttackBase::TraceAndDamageTick()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	APawn* AvatarPawn = Cast<APawn>(AvatarActor);
	if (!AvatarPawn) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarActor);
	// 발사 지점: 가슴이나 어깨 쪽 소켓 권장 (더 정면 느낌 남)
	const FVector SpawnLocation = CombatInterface ? CombatInterface->GetSocketLocation(SocketTag) : AvatarActor->GetActorLocation();

	FVector ViewLocation; FRotator ViewRotation;
	AvatarPawn->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// --- [내가 추천하는 각도 제한] ---
	FRotator FixedRotation = ViewRotation;
	// 위로는 -45도(꽤 높음)까지 가능, 아래로는 10도(무릎 정도)까지만!
	// 이렇게 하면 바닥 조준해도 빔은 정면 적을 향해 날아감.
	FixedRotation.Pitch = FMath::ClampAngle(FixedRotation.Pitch, -45.0f, 10.0f);
	// -------------------------------

	// 트레이스 시작점은 카메라에서 살짝 앞으로 밀어줌 (자기 몸 통과 방지)
	FVector TraceStart = ViewLocation + (FixedRotation.Vector() * 100.f); 
	FVector TraceEnd = TraceStart + (FixedRotation.Vector() * BeamRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarPawn);

	// 빔 두께를 고려한 Sphere Trace (반지름 40 정도면 넉넉함)
	float BeamRadius = 40.f; 
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult, 
		TraceStart, 
		TraceEnd, 
		FQuat::Identity, 
		ECC_Pawn, 
		FCollisionShape::MakeSphere(BeamRadius), 
		QueryParams
	);

	FVector TargetLocation = bHit ? HitResult.Location : TraceEnd;

	if (AvatarActor->HasAuthority())
	{
		if (bHit && HitResult.GetActor())
		{
			// 적 맞췄을 때 로그 (데미지 확인용)
			UE_LOG(LogTemp, Warning, TEXT("!!! Beam Hit Target: %s !!!"), *HitResult.GetActor()->GetName());

			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor()))
			{
				if (DamageEffectClass)
				{
					FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
					if (DamageSpec.IsValid()) TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
				}
			}
		}

		// [보라색 선] 이제 조준을 내려도 이 선은 바닥에 안 박히고 정면을 유지할 거야
		DrawDebugLine(GetWorld(), SpawnLocation, TargetLocation, FColor::Magenta, false, 0.1f, 0, 8.0f);
	}
}
void UBeamAttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BeamTimerHandle);
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveGameplayCue(BeamCueTag);
	}

	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.1f, BeamMontage);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}