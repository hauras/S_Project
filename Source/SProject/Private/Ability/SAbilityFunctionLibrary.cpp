

#include "Ability/SAbilityFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameMode/SGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Ability/SAttributeSet.h" // SAttributeSet 헤더를 include 해야 합니다.

void USAbilityFunctionLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, EEnemyType EnemyType,
                                                           float Level, UAbilitySystemComponent* ASC)
{
	// ===================== 디버그 강화 버전 시작 =====================

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!AvatarActor)
	{
		UE_LOG(LogTemp, Error, TEXT("[DEBUG] InitializeDefaultAttributes FAILED: AvatarActor is NULL."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("===== [DEBUG] Initializing Attributes for [%s] ====="), *AvatarActor->GetName());

	const ASGameMode* SGameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SGameMode == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[DEBUG] InitializeDefaultAttributes FAILED: SGameMode is NULL."));
		return;
	}

	UEnemyStats* EnemyInfo = SGameMode->EnemyInfo;
	if (EnemyInfo == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[DEBUG] InitializeDefaultAttributes FAILED: EnemyInfo Data Asset is NOT SET in the GameMode!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Using EnemyStats Data Asset: [%s]"), *EnemyInfo->GetName());

	// 스탯 변경 전의 값을 먼저 확인합니다.
	const USAttributeSet* AS = Cast<USAttributeSet>(ASC->GetAttributeSet(USAttributeSet::StaticClass()));
	if (AS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Health BEFORE apply: [%f]"), AS->GetHealth());
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(AvatarActor);

	// 1. MaxVitalAttributes 적용 시도
	if (EnemyInfo->MaxVitalAttributes)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Applying MaxVitalAttributes GE: [%s]"), *EnemyInfo->MaxVitalAttributes->GetName());
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EnemyInfo->MaxVitalAttributes, Level, ContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DEBUG] MaxVitalAttributes GE is NOT SET in the Data Asset!"));
	}

	// 2. VitalAttributes 적용 시도
	if (EnemyInfo->VitalAttributes)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Applying VitalAttributes GE: [%s]"), *EnemyInfo->VitalAttributes->GetName());
		const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EnemyInfo->VitalAttributes, Level, ContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DEBUG] VitalAttributes GE is NOT SET in the Data Asset!"));
	}
	
	// 모든 효과 적용 후의 최종 값을 확인합니다.
	if (AS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Health AFTER apply: [%f]"), AS->GetHealth());
	}
	UE_LOG(LogTemp, Warning, TEXT("===== [DEBUG] Initialization Finished for [%s] ====="), *AvatarActor->GetName());
	// ===================== 디버그 강화 버전 끝 =====================
}

void USAbilityFunctionLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC)
{
	const ASGameMode* SGameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(UGameplayStatics::GetGameMode(WorldContextObject)));
	UEnemyStats* EnemyInfo = SGameMode->EnemyInfo;
	for (TSubclassOf<UGameplayAbility> AbilityClass : EnemyInfo->Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
}

UEnemyStats* USAbilityFunctionLibrary::GetMonsterClassInfo(const UObject* WorldContextObject)
{
	ASGameMode* SGameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SGameMode == nullptr) return nullptr;
	return SGameMode->EnemyInfo;
}
