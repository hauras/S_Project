

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

	const FVector SocketLocation = CombatInterface->GetSocketLocation(SocketRequestTag);
		
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());

	TArray<AActor*> OverlappingActors;

	USAbilityFunctionLibrary::GetLivePlayersWithinRadius(
		this,                 
		OverlappingActors,
		ActorsToIgnore,
		DamageRadius,          
		SocketLocation
	);

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

					TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
				}
			}
		}
	}
}
