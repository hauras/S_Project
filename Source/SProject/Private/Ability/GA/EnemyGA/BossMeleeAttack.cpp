

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

	// ★★★ 1. '어떤 주먹'으로 때렸는지 판단할 변수를 만듭니다. ★★★
	FGameplayTag SocketRequestTag;

	// ★★★ 2. 애님 노티파이로부터 전달된 '이벤트 태그'를 확인합니다. ★★★
	if (Payload.EventTag.MatchesTagExact(FSGameplayTags::Get().Event_Montage_Attack_Fist_Right))
	{
		// "오른 주먹 이벤트"라면, '오른 주먹 소켓' 태그를 사용합니다.
		SocketRequestTag = FSGameplayTags::Get().Combat_Socket_Fist_Right;
	}
	else if (Payload.EventTag.MatchesTagExact(FSGameplayTags::Get().Event_Montage_Attack_Fist_Left))
	{
		// "왼 주먹 이벤트"라면, '왼 주먹 소켓' 태그를 사용합니다.
		SocketRequestTag = FSGameplayTags::Get().Combat_Socket_Fist_Left;
	}
	else
	{
		// 약속된 이벤트가 아니면 아무것도 하지 않고 종료합니다.
		return;
	}

	// 3. 알아낸 '주먹 태그'를 사용하여 정확한 소켓 위치를 가져옵니다.
	const FVector SocketLocation = CombatInterface->GetSocketLocation(SocketRequestTag);

	// 4. 무시할 액터 목록을 만듭니다. (시전자 자신)
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());

	// 5. 탐지 결과를 담을 빈 배열을 만듭니다.
	TArray<AActor*> OverlappingActors;

	// 6. '만능 탐지기'를 호출하여, 해당 주먹의 위치와 반경 내의 살아있는 모든 적을 찾아냅니다.
	USAbilityFunctionLibrary::GetLivePlayersWithinRadius(
		this,                  // World Context
		OverlappingActors,
		ActorsToIgnore,
		180.f,          // DamageRadius는 헤더에 선언된 UPROPERTY 변수여야 합니다.
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
			}
		}
	}
}
