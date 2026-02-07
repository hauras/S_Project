

#include "UI/WidgetController/OverlayWidgetController.h"

#include "Ability/SAbilitySystemComponent.h"
#include "Ability/SAttributeSet.h"
#include "Data/AbilityInfo.h"
#include "Data/PlayerDataAsset.h"
#include "State/SPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const USAttributeSet* SAttributeSet = Cast<USAttributeSet>(AttributeSet);
	
	OnHealthChanged.Broadcast(SAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(SAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(SAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(SAttributeSet->GetMaxMana());

	if (USAbilitySystemComponent* SASC = Cast<USAbilitySystemComponent>(AbilitySystemComponent))
	{
		if (SASC->bStartupAbilitiesGiven)
		{
			OnInitializeStartupAbilities(SASC);
		}
	}
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

	USAbilitySystemComponent* SASC = Cast<USAbilitySystemComponent>(AbilitySystemComponent);
	ASPlayerState* PS = Cast<ASPlayerState>(PlayerState);

	if (SASC && PS)
	{
		PS->OnCharacterIndexChangedDelegate.AddLambda([this, SASC](int32 NewIndex)
		{
			OnInitializeStartupAbilities(SASC);
		});

		SASC->AbilityGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);

		if (SASC->bStartupAbilitiesGiven)
		{
			OnInitializeStartupAbilities(SASC);
		}
	}
}

void UOverlayWidgetController::OnInitializeStartupAbilities(USAbilitySystemComponent* SAbilitySystemComponent)
{
	if (!SAbilitySystemComponent->bStartupAbilitiesGiven) return;

	ASPlayerState* PS = Cast<ASPlayerState>(PlayerState);
	if (!PS || !PS->PlayerData.IsValidIndex(PS->CurrentCharacterIndex)) return;
	
	UPlayerDataAsset* CurrentData = PS->PlayerData[PS->CurrentCharacterIndex];
	UAbilityInfo* CurrentHeroAbilityInfo = CurrentData->CharacterInfo.AbilityInfo;

	bool bAnySkillMatched = false;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, SAbilitySystemComponent, CurrentHeroAbilityInfo, &bAnySkillMatched](const FGameplayAbilitySpec& AbilitySpec)
	{
		FGameplayTag AbilityTag = SAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec);
		FSAbilityInfo Info = CurrentHeroAbilityInfo->FindAbilityInfoForTag(AbilityTag);
		
		if (Info.AbilityTag.IsValid())
		{
			bAnySkillMatched = true;
			Info.InputTag = SAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(Info);
		}
	});
	
	SAbilitySystemComponent->ForEachAbility(BroadcastDelegate);

	if (!bAnySkillMatched)
	{
		FTimerHandle RetryTimer;
		GetWorld()->GetTimerManager().SetTimer(RetryTimer, [this, SAbilitySystemComponent]()
		{
			OnInitializeStartupAbilities(SAbilitySystemComponent);
		}, 0.1f, false);
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