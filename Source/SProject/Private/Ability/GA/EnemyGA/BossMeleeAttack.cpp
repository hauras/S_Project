

#include "Ability/GA/EnemyGA/BossMeleeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "SGameplayTags.h"
#include "Ability/SAbilityFunctionLibrary.h"
#include "Interface/CombatInterface.h"
#include "AbilitySystemComponent.h"

void UBossMeleeAttack::HandleDamageEvent(FGameplayEventData Payload)
{
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface == nullptr) return;

	FGameplayTag SocketRequestTag;

	// ★★★ 2. 애님 노티파이로부터 전달된 '이벤트 태그'를 확인합니다. ★★★
	if (Payload.EventTag.MatchesTagExact(FSGameplayTags::Get().Event_Montage_Attack_Fist_Right))
	{
		SocketRequestTag = FSGameplayTags::Get().Combat_Socket_Fist_Right;
	}
	else if (Payload.EventTag.MatchesTagExact(FSGameplayTags::Get().Event_Montage_Attack_Fist_Left))
	{
		SocketRequestTag = FSGameplayTags::Get().Combat_Socket_Fist_Left;

	}
	else
	{
		return;
	}

	// 3. 알아낸 '주먹 태그'를 사용하여 정확한 소켓 위치를 가져옵니다.
	const FVector SocketLocation = CombatInterface->GetSocketLocation(SocketRequestTag);
		
	// 4. 무시할 액터 목록을 만듭니다. (시전자 자신)
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());

	// 5. 탐지 결과를 담을 빈 배열을 만듭니다.
	TArray<AActor*> OverlappingActors;

	USAbilityFunctionLibrary::GetLivePlayersWithinRadius(
		this,                  // World Context
		OverlappingActors,
		ActorsToIgnore,
		DamageRadius,          // DamageRadius는 헤더에 선언된 UPROPERTY 변수여야 합니다.
		SocketLocation
	);

	// 7. 찾아낸 모든 타겟에게 데미지를 적용합니다.
	if (OverlappingActors.Num() > 0 && DamageEffectClass)
	{
		FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		for (AActor* TargetActor : OverlappingActors)
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
			{
				TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());

				FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
				FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(StateEffectClass, GetAbilityLevel(), ContextHandle);

				if (EffectSpecHandle.IsValid()) 
				{
					UE_LOG(LogTemp, Warning, TEXT("Attempting to apply Stun to Target!")); // 이 로그가 뜨는지 확인

					TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
				}
			}
		}
	}
}
