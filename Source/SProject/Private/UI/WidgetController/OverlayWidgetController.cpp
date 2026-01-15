

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
		// 2. "혹시 스킬 부여가 이미 끝났나요?" 라고 확인합니다.
		if (SASC->bStartupAbilitiesGiven)
		{
			// 3. 만약 끝났다면, 스킬 정보를 방송하는 함수를 '다시 한번' 호출해 줍니다.
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

	// 1. 현재 플레이어의 영혼(PlayerState)을 가져옵니다.
	ASPlayerState* PS = Cast<ASPlayerState>(PlayerState);
	if (!PS) return;

	// 2. [핵심 ⭐] 현재 조종 중인 캐릭터의 '데이터 에셋'에서 '전용 스킬 사전'을 꺼냅니다.
	// PS에 있는 인덱스를 통해 현재 캐릭터가 누구인지 찾습니다.
	if (!PS->PlayerData.IsValidIndex(PS->CurrentCharacterIndex)) return;
	
	UPlayerDataAsset* CurrentData = PS->PlayerData[PS->CurrentCharacterIndex];
	if (!CurrentData || !CurrentData->CharacterInfo.AbilityInfo) return;

	// 이 캐릭터만을 위한 스킬 UI 정보 사전입니다.
	UAbilityInfo* CurrentHeroAbilityInfo = CurrentData->CharacterInfo.AbilityInfo;

	FForEachAbility BroadcastDelegate;
	// 3. 람다 함수 캡처 리스트에 'CurrentHeroAbilityInfo'를 추가하여 
	// 루프 안에서 이 사전을 사용하도록 합니다.
	BroadcastDelegate.BindLambda([this, SAbilitySystemComponent, CurrentHeroAbilityInfo](const FGameplayAbilitySpec& AbilitySpec)
	{
		// [수정된 부분] 고정된 'AbilityInfo' 대신 '현재 영웅의 사전'에서 정보를 찾습니다.
		FSAbilityInfo Info = CurrentHeroAbilityInfo->FindAbilityInfoForTag(SAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		
		Info.InputTag = SAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	
	SAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
}
