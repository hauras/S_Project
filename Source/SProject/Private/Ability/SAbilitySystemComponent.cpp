

#include "Ability/SAbilitySystemComponent.h"
#include "SGameplayTags.h"
#include "Ability/GA/SGameplayAbility.h"

void USAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> Ability : StartupAbilities)
	{
		
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, 1);
		if (const USGameplayAbility* SAbility = Cast<USGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(SAbility->InputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

void USAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void USAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// [검문소 2] ASC가 신호를 받았는지 확인
	UE_LOG(LogTemp, Warning, TEXT("DEBUG: 2. [ASC] Received Tag: %s. Checking Abilities..."), *InputTag.ToString());
	
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 현재 검사 중인 어빌리티 이름과 가지고 있는 태그들을 모두 출력해 봅니다.
		// 이 로그를 통해 어빌리티에 태그가 제대로 들어갔는지 눈으로 확인할 수 있습니다.
		UE_LOG(LogTemp, Log, TEXT("DEBUG:    - Checking Ability: %s | Tags: %s"), *AbilitySpec.Ability->GetName(), *AbilitySpec.DynamicAbilityTags.ToStringSimple());

		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// [검문소 3] 태그가 일치하는 어빌리티를 찾았는지 확인
			UE_LOG(LogTemp, Error, TEXT("DEBUG: 3. [ASC] MATCH FOUND! Trying to activate: %s"), *AbilitySpec.Ability->GetName());

			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				if (!TryActivateAbility(AbilitySpec.Handle))
				{
					// 활성화 시도했는데 실패한 경우 (예: 코스트 부족, 쿨다운 중, 태그로 막힘)
					UE_LOG(LogTemp, Error, TEXT("DEBUG: [ASC] TryActivateAbility FAILED for %s"), *AbilitySpec.Ability->GetName());
				}
			}
		}
	}
}

void USAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}
