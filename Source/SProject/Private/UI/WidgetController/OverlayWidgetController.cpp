

#include "UI/WidgetController/OverlayWidgetController.h"

#include "Ability/SAbilitySystemComponent.h"
#include "Ability/SAttributeSet.h"
#include "Data/AbilityInfo.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const USAttributeSet* SAttributeSet = Cast<USAttributeSet>(AttributeSet);
	
	OnHealthChanged.Broadcast(SAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(SAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(SAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(SAttributeSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const USAttributeSet* SAttributeSet = Cast<USAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		SAttributeSet->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		SAttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		SAttributeSet->GetManaAttribute()).AddUObject(this, &UOverlayWidgetController::ManaChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		SAttributeSet->GetMaxManaAttribute()).AddUObject(this, &UOverlayWidgetController::MaxManaChanged);

	if (USAbilitySystemComponent* SASC = Cast<USAbilitySystemComponent>(AbilitySystemComponent))
	{
		if (SASC->bStartupAbilitiesGiven)
		{
			OnInitializeStartupAbilities(SASC);
		}
		else
		{
			SASC->AbilityGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);
		}
	}
}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data)
{

	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data)
{

	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data)
{
	OnManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data)
{
	OnMaxManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::OnInitializeStartupAbilities(USAbilitySystemComponent* SAbilitySystemComponent)
{
	if (!SAbilitySystemComponent->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, SAbilitySystemComponent](const FGameplayAbilitySpec& AbilitySpec)
	{
		
		FSAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(SAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = SAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	SAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
}
