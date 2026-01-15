
#include "Ability/SAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "SGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BossCharacter.h"
#include "Controller/SPlayerController.h"
#include "GameFramework/Character.h"
#include "Interface/CombatInterface.h"
#include "Kismet/GameplayStatics.h"

USAttributeSet::USAttributeSet()
{

}

void USAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);

}

void USAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void USAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	bool bHealthChanged = false;
	const float LocalIncomingDamage = GetIncomingDamage();

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		
		SetIncomingDamage(0.f);

		if (LocalIncomingDamage > 0.f)
		{
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			
			bHealthChanged = true; 
		}
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		bHealthChanged = true;
	}

	if (bHealthChanged)
	{
		const float CurrentHealth = GetHealth();
		const float CurrentMaxHealth = GetMaxHealth();

		// A. 사망 판정 
		if (CurrentHealth <= 0.f)
		{
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
			{
				CombatInterface->Die();
			}
			return; 
		}

		// B. 보스
		if (ABossCharacter* Boss = Cast<ABossCharacter>(Props.TargetAvatarActor))
		{
			Boss->PlayHitReactEffect();

			// 2) 소환 페이즈 체크 (체력 50% 이하 & 아직 소환 전)
			if (!Boss->bHasSummoned && CurrentMaxHealth > 0.f)
			{
				float HealthPercent = CurrentHealth / CurrentMaxHealth;
				if (HealthPercent <= 0.5f)
				{
					Boss->bHasSummoned = true;
					
					if (AAIController* AIC = Cast<AAIController>(Boss->GetController()))
					{
						if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
						{
							BB->SetValueAsBool(FName("WantToSummon"), true);
						}
					}
				}
			}
		}
		// C. 타겟이 일반 몬스터나 플레이어인 경우
		else
		{
			bool bIsStunned = Props.TargetASC->HasMatchingGameplayTag(FSGameplayTags::Get().State_Stun);

			// 2. 기절 상태가 아닐 때
			if (!bIsStunned)
			{
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FSGameplayTags::Get().Ability_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
			}
		}

		ShowFloatingText(Props, LocalIncomingDamage);
	}
}

void USAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void USAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage) const
{
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		if (ASPlayerController* PC = Cast<ASPlayerController>(UGameplayStatics::GetPlayerController(Props.SourceCharacter, 0)))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter);
		}
	}
}

void USAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USAttributeSet, Health, OldHealth);
}

void USAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USAttributeSet, MaxHealth, OldMaxHealth);
}

void USAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USAttributeSet, Mana, OldMana);
}

void USAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USAttributeSet, MaxMana, OldMaxMana);
}

void USAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USAttributeSet, AttackPower, OldAttackPower);
}

