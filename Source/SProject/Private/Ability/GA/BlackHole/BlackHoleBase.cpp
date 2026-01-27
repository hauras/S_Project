

#include "Ability/GA/BlackHole/BlackHoleBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "DrawDebugHelpers.h" // 헤더 상단에 추가

UBlackHoleBase::UBlackHoleBase()
{
}

void UBlackHoleBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BlackHoleTag);
	if (EventTask) 
	{
		EventTask->EventReceived.AddDynamic(this, &UBlackHoleBase::OnEventReceived);
		EventTask->ReadyForActivation();
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AnimMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UBlackHoleBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UBlackHoleBase::OnMontageEnded);
		MontageTask->OnCancelled.AddDynamic(this, &UBlackHoleBase::OnMontageEnded);
		MontageTask->ReadyForActivation();

	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	
}



void UBlackHoleBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

}

void UBlackHoleBase::OnEventReceived(FGameplayEventData Payload)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	APawn* AvatarPawn = Cast<APawn>(AvatarActor);

	FVector ViewLocation;
	FRotator ViewRotation;
	AvatarPawn->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector TraceEnd = ViewLocation + (ViewRotation.Vector() * SpawnDistance);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(AvatarActor);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility, CollisionParams);

	FVector SpawnLocation = bHit ? HitResult.Location : TraceEnd;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = AvatarPawn;

	AActor* SpawnedBlackHole = GetWorld()->SpawnActor<AActor>(
		BlackHoleClass,  
		SpawnLocation,   
		FRotator::ZeroRotator,
		SpawnParams      
	);
	
}

void UBlackHoleBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}