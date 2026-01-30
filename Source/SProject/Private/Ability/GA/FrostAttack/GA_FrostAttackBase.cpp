

#include "Ability/GA/FrostAttack/GA_FrostAttackBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/CharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UGA_FrostAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Character) || !IsValid(FrostAttackMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FrostAttackMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_FrostAttackBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_FrostAttackBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}

	const FVector SpawnLocation = Character->GetActorLocation();

	// 기존 '중심부' 이펙트 생성 코드는 그대로 둡니다.
	if (FrostParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FrostParticle, SpawnLocation);
	}

	if (ImpactParticle && ImpactParticle_Num > 0)
	{
		const float Radius = AttackRadius; 
		const float AngleIncrement = 360.f / ImpactParticle_Num;

		for (int32 i = 0; i < ImpactParticle_Num; ++i)
		{
			const float CurrentAngle = AngleIncrement * i;
			const float Radian = FMath::DegreesToRadians(CurrentAngle);
			const float OffsetX = Radius * FMath::Cos(Radian);
			const float OffsetY = Radius * FMath::Sin(Radian);
			
			FVector LocationInAir = SpawnLocation + FVector(OffsetX, OffsetY, 0.f);

			FVector TraceStart = LocationInAir;
			FVector TraceEnd = LocationInAir - FVector(0.f, 0.f, 1000.f);

			FHitResult HitResult;
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(Character);
			
			UKismetSystemLibrary::LineTraceSingle(
				GetWorld(),
				TraceStart,
				TraceEnd,
				UEngineTypes::ConvertToTraceType(ECC_Visibility), 
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None, 
				HitResult,
				true
			);

			FVector FinalImpactLocation = LocationInAir; 
			if (HitResult.bBlockingHit)
			{
				FinalImpactLocation = HitResult.ImpactPoint;
			}

			FRotator ImpactRotation = FRotator(0.f, CurrentAngle, 0.f);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, FinalImpactLocation, ImpactRotation);
		}
	}

	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		SpawnLocation,
		AttackRadius,
		ObjectTypes,
		ACharacterBase::StaticClass(),
		IgnoreActors,
		OverlappingActors
	);	

	if (OverlappingActors.Num() > 0)
	{
		if (DamageEffectClass)
		{
			FGameplayEffectContextHandle MainContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
			MainContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
			MainContext.AddSourceObject(this);

			FGameplayEffectSpecHandle DamageSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), MainContext);
		
			for (AActor* TargetActor : OverlappingActors)
			{
				ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);

				if (CombatInterface && !CombatInterface->IsDead())
				{
					if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());

						ExecuteSynergyLogic(TargetActor); 
					}
				}
			}
		}
	}
	
}

void UGA_FrostAttackBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);

}
