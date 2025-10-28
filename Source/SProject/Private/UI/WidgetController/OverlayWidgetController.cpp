

#include "UI/WidgetController/OverlayWidgetController.h"

#include "Ability/SAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const USAttributeSet* SAttributeSet = Cast<USAttributeSet>(AttributeSet);

	UE_LOG(LogTemp, Error, TEXT("DEBUG: [WController] Broadcasting Initial Values! Health: %f, MaxHealth: %f"), SAttributeSet->GetHealth(), SAttributeSet->GetMaxHealth());
	
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
}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUG: [WController] Health Changed event received! NewValue: %f"), Data.NewValue);

	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUG: [WController] MaxHealth Changed event received! NewValue: %f"), Data.NewValue);

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
