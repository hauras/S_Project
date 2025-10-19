
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API USAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:

	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
};
