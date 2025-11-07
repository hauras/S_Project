#include "Ability/SAbilityFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameMode/SGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Ability/SAttributeSet.h"

void USAbilityFunctionLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, EEnemyType EnemyType,
                                                           float Level, UAbilitySystemComponent* ASC)
{
	if (!IsValid(ASC) || !IsValid(ASC->GetAvatarActor()))
	{
		return;
	}

	const ASGameMode* SGameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SGameMode == nullptr)
	{
		return;
	}

	UEnemyStats* EnemyInfo = SGameMode->EnemyInfo;
	if (EnemyInfo == nullptr)
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(ASC->GetAvatarActor());

	if (EnemyInfo->MaxVitalAttributes)
	{
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EnemyInfo->MaxVitalAttributes, Level, ContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	if (EnemyInfo->VitalAttributes)
	{
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EnemyInfo->VitalAttributes, Level, ContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void USAbilityFunctionLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC)
{
	const ASGameMode* SGameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SGameMode && SGameMode->EnemyInfo)
	{
		for (TSubclassOf<UGameplayAbility> AbilityClass : SGameMode->EnemyInfo->Abilities)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UEnemyStats* USAbilityFunctionLibrary::GetMonsterClassInfo(const UObject* WorldContextObject)
{
	ASGameMode* SGameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SGameMode == nullptr)
	{
		return nullptr;
	}
	return SGameMode->EnemyInfo;
}