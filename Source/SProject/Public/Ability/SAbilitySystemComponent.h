
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FAbilityGiven, USAbilitySystemComponent*);
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);
/**
 * 
 */
UCLASS()
class SPROJECT_API USAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:

	FAbilityGiven AbilityGivenDelegate;
	
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

	UPROPERTY(ReplicatedUsing = OnRep_StartupAbilitiesGiven)
	bool bStartupAbilitiesGiven = false;

	UFUNCTION()
	void OnRep_StartupAbilitiesGiven();
	
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void ForEachAbility(const FForEachAbility& Delegate);

	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

protected:

};
