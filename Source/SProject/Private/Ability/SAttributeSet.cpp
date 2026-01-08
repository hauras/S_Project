
#include "Ability/SAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "SGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BossCharacter.h"
#include "GameFramework/Character.h"
#include "Interface/CombatInterface.h"

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

	// 1. 필요한 정보들(Source, Target 액터 및 컨트롤러 등)을 추출합니다.
	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	// 체력이 변했는지 여부를 추적할 플래그
	bool bHealthChanged = false;

	// [경로 A] 데미지 계산기(ExecCalc)를 통해 'IncomingDamage'가 들어온 경우
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		
		// 주머니를 즉시 0으로 비웁니다. (다음 공격 계산을 위해 필수!)
		SetIncomingDamage(0.f);

		if (LocalIncomingDamage > 0.f)
		{
			// 현재 체력에서 데미지를 뺀 새로운 체력 계산
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			
			// 0~MaxHealth 사이로 안전하게 고정(Clamp)하여 적용
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			
			bHealthChanged = true; // 체력이 변했음을 표시
		}
	}

	// [경로 B] 포션 사용 등으로 'Health' 어트리뷰트가 직접 변한 경우
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		bHealthChanged = true;
	}

	// ---------------------------------------------------
	// 2. 체력이 변했을 때만 실행되는 공통 부가 로직 (사망/피격/페이즈)
	// ---------------------------------------------------
	if (bHealthChanged)
	{
		const float CurrentHealth = GetHealth();
		const float CurrentMaxHealth = GetMaxHealth();

		// A. 사망 판정 (가장 최우선 순위)
		if (CurrentHealth <= 0.f)
		{
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
			{
				CombatInterface->Die();
			}
			return; // 사망 시 아래 로직(피격 애니메이션 등)은 무시하고 종료
		}

		// B. 타겟이 '보스'인 경우의 특별 처리
		if (ABossCharacter* Boss = Cast<ABossCharacter>(Props.TargetAvatarActor))
		{
			// 1) 보스 전용 피격 번쩍임 효과 재생
			Boss->PlayHitReactEffect();

			// 2) 소환 페이즈 체크 (체력 50% 이하 & 아직 소환 전)
			if (!Boss->bHasSummoned && CurrentMaxHealth > 0.f)
			{
				float HealthPercent = CurrentHealth / CurrentMaxHealth;
				if (HealthPercent <= 0.5f)
				{
					Boss->bHasSummoned = true;
					
					// AI 컨트롤러를 찾아 블랙보드 값을 바꿉니다.
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
			// 기존 애니메이션 기반 HitReact 어빌리티 발동
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(FSGameplayTags::Get().Ability_HitReact);
			Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
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
