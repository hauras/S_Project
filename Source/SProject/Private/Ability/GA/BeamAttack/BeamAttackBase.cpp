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
	
	UAbilityTask_WaitInputRelease* InputWaitTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (InputWaitTask)
	{
		InputWaitTask->OnRelease.AddDynamic(this, &UBeamAttackBase::OnInputReleased);
		InputWaitTask->ReadyForActivation();
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, FName("Start"), BeamMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UBeamAttackBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UBeamAttackBase::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UBeamAttackBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}

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

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.EffectCauser = GetAvatarActorFromActorInfo(); 
		CueParams.Location = GetAvatarActorFromActorInfo()->GetActorLocation(); 
        
		ASC->AddGameplayCue(BeamCueTag, CueParams);
	}

	GetWorld()->GetTimerManager().SetTimer(BeamTimerHandle, this, &UBeamAttackBase::TraceAndDamageTick, TickRate, true);
}

void UBeamAttackBase::TraceAndDamageTick()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	APawn* AvatarPawn = Cast<APawn>(AvatarActor);
	if (!AvatarPawn) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarActor);
	const FVector SpawnLocation = CombatInterface ? CombatInterface->GetSocketLocation(SocketTag) : AvatarActor->GetActorLocation();

	FVector ViewLocation; FRotator ViewRotation;
	AvatarPawn->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FRotator FixedRotation = ViewRotation;
	FixedRotation.Pitch = FMath::ClampAngle(FixedRotation.Pitch, -45.0f, 10.0f);

	FVector TraceStart = ViewLocation + (FixedRotation.Vector() * 100.f); 
	FVector TraceEnd = TraceStart + (FixedRotation.Vector() * BeamRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarPawn);

	AActor* HitEnemy = HitResult.GetActor(); // 이게 바로 그 녀석!
    
	// 시너지 로직 실행 (방금 만든 부모 함수 호출)
	ExecuteSynergyLogic(HitEnemy);
	
	float BeamRadius = 50.f; 
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

			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor()))
			{
				if (DamageEffectClass)
				{
					FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
					if (DamageSpec.IsValid()) TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
				}
			}
		}

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