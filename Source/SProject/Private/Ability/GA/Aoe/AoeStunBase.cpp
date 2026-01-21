

#include "Ability/GA/Aoe/AoeStunBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

void UAoeStunBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Player) || !IsValid(FlareMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FlareMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UAoeStunBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UAoeStunBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FlareTag); // 헤더의 변수 이름과 통일 (ProjectileTag -> FireEventTag)
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UAoeStunBase::OnFlareEventReceived);
		EventTask->ReadyForActivation();
	}
}

void UAoeStunBase::OnFlareEventReceived(FGameplayEventData Payload)
{

	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	TArray<AActor*> OutActors;
	TArray<AActor*> ActorsToIgnore;
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	ActorsToIgnore.Add(AvatarActor);
	FVector ActorLoc = AvatarActor->GetActorLocation();

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		ActorLoc,
		FlareRadius,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OutActors
	);

	if (FlareParticle) 
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FlareParticle, ActorLoc, FRotator::ZeroRotator, FVector(FlareRadius / 100.f));
	}
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	for (AActor* Target : OutActors)
	{
		ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target); 

		if (CombatInterface && !CombatInterface->IsDead())
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
			if (TargetASC)
			{
				if (DamageEffectClass)
				{
					FGameplayEffectSpecHandle DamageSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);
					if (DamageSpecHandle.IsValid())
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
					}
				}

				// 2. [기절 시간 계산] 질문자님의 알고리즘 유지
				float Distance = Target->GetDistanceTo(AvatarActor);
				float MaxDuration = 3.0f; 
				float MinDuration = 0.2f; 

				float Alpha = FMath::Clamp(Distance / FlareRadius, 0.f, 1.f);
				float ExponentialAlpha = Alpha * Alpha; // 지수 감쇄 적용
				float StunDuration = FMath::Lerp(MaxDuration, MinDuration, ExponentialAlpha);

				// 3. [기절 배달] 
				if (FlareEffectClass)
				{
					FGameplayEffectSpecHandle StunSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(FlareEffectClass, GetAbilityLevel(), ContextHandle);
					if (StunSpecHandle.IsValid())
					{
						// SetByCaller 방식으로 계산된 시간을 주입
						UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
							StunSpecHandle,
							FGameplayTag::RequestGameplayTag(FName("Data.Damage.Duration")),
							StunDuration
						);       
					
						TargetASC->ApplyGameplayEffectSpecToSelf(*StunSpecHandle.Data.Get());
					
						UE_LOG(LogTemp, Log, TEXT("%s 에게 데미지 적용 및 %.2f초 스턴!"), *Target->GetName(), StunDuration);
					}
				}
			}
		}
	}
	// 추후 여기에 나이아가라 폭발 이펙트 소환 코드를 넣으세요!
}

void UAoeStunBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

}
