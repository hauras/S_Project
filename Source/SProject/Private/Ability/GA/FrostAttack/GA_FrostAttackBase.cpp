

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
			
			// 1. 캐릭터 허리 높이의 '공중 위치'를 먼저 계산합니다.
			FVector LocationInAir = SpawnLocation + FVector(OffsetX, OffsetY, 0.f);

			// 2. 바닥을 찾기 위해 라인 트레이스를 시작할 지점과 끝 지점을 설정합니다.
			// 공중 위치에서 시작해서, 그보다 1000 유닛 아래까지 레이저를 쏩니다.
			FVector TraceStart = LocationInAir;
			FVector TraceEnd = LocationInAir - FVector(0.f, 0.f, 1000.f);

			// 3. 라인 트레이스를 실행합니다.
			FHitResult HitResult;
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(Character); // 자기 자신은 무시
			
			UKismetSystemLibrary::LineTraceSingle(
				GetWorld(),
				TraceStart,
				TraceEnd,
				UEngineTypes::ConvertToTraceType(ECC_Visibility), // 눈에 보이는 모든 것과 충돌
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None, // 디버그 라인을 보려면 ForDuration으로 변경
				HitResult,
				true
			);

			// 4. 바닥을 찾았는지 확인하고, 최종 생성 위치를 결정합니다.
			FVector FinalImpactLocation = LocationInAir; // 기본값은 공중 위치
			if (HitResult.bBlockingHit)
			{
				// 바닥을 찾았다면, 레이저가 닿은 바로 그 지점을 최종 위치로 설정합니다.
				FinalImpactLocation = HitResult.ImpactPoint;
			}

			// 5. 계산된 최종 위치에 이펙트를 생성합니다.
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
			FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
			
			// ★★★ 2. for 루프를 아래와 같이 수정합니다. ★★★
			for (AActor* TargetActor : OverlappingActors)
			{
				// 타겟을 ICombatInterface로 캐스팅해 봅니다.
				ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);

				// 캐스팅에 성공했고, 그 타겟이 아직 죽지 않았다면...
				if (CombatInterface && !CombatInterface->IsDead())
				{
					// ...그때만 데미지를 적용합니다.
					if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
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
