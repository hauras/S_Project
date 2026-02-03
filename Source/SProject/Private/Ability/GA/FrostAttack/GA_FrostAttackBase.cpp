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

	// 1. 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FrostAttackMontage);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGA_FrostAttackBase::OnMontageEnded);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_FrostAttackBase::OnMontageEnded);
		MontageTask->ReadyForActivation();
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const FVector SpawnLocation = Character->GetActorLocation();

	// 2. [중심부 이펙트] GameplayCue로 전환
	if (FrostCueTag.IsValid())
	{
		FGameplayCueParameters Params;
		Params.Location = SpawnLocation;
		Params.Instigator = Character;
		
		// 서버에서 호출 시 모든 클라이언트에게 전파됨
		ASC->ExecuteGameplayCue(FrostCueTag, Params);
	}

	// 3. [주변 파편 이펙트] 루프 돌며 GameplayCue 실행
	if (ImpactParticle_Num > 0 && ImpactCueTag.IsValid())
	{
		const float Radius = AttackRadius; 
		const float AngleIncrement = 360.f / ImpactParticle_Num;

		for (int32 i = 0; i < ImpactParticle_Num; ++i)
		{
			// 1. [기존 수학 로직 그대로 사용] 주변 좌표 계산
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
				GetWorld(), TraceStart, TraceEnd,
				UEngineTypes::ConvertToTraceType(ECC_Visibility), 
				false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true
			);

			// 2. 최종 얼음이 생길 위치(바닥 지점) 확정
			FVector FinalImpactLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : LocationInAir;

			// 3. [중요] 계산된 각 지점마다 GameplayCue를 실행!
			FGameplayCueParameters Params;
			Params.Location = FinalImpactLocation; // 계산된 주변 좌표를 넣어줍니다.
			Params.Instigator = Character;
			// 회전값 정보를 넘겨주고 싶다면 Normal이나 Magnitude를 활용할 수 있습니다.
			Params.Normal = HitResult.ImpactNormal; 

			// 8~16번 루프를 돌며 각 위치에서 큐가 터집니다.
			ASC->ExecuteGameplayCue(ImpactCueTag, Params);
		}
	}

	// 4. [범위 데미지 및 시너지 판정]
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

	if (OverlappingActors.Num() > 0 && DamageEffectClass)
	{
		FGameplayEffectContextHandle MainContext = ASC->MakeEffectContext();
		MainContext.AddInstigator(Character, Character);
		MainContext.AddSourceObject(this);

		FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		
		for (AActor* TargetActor : OverlappingActors)
		{
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);

			if (CombatInterface && !CombatInterface->IsDead())
			{
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
				{
					// 데미지 적용
					TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());

					// 캐릭터 간 시너지 로직 (부모 클래스 함수)
					ExecuteSynergyLogic(TargetActor); 
				}
			}
		}
	}
}

void UGA_FrostAttackBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}