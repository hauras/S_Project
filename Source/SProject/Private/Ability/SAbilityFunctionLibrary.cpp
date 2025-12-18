#include "Ability/SAbilityFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "GameMode/SGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Ability/SAttributeSet.h"
#include "Interface/CombatInterface.h"
#include "Engine/OverlapResult.h" 

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

void USAbilityFunctionLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, EEnemyType EnemyType)
{
	// 1. 게임 모드와 EnemyStats 데이터 애셋을 가져옵니다. (기존과 동일)
	const ASGameMode* SGameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SGameMode == nullptr || SGameMode->EnemyInfo == nullptr)
	{
		return;
	}
	UEnemyStats* EnemyInfo = SGameMode->EnemyInfo;

	// ★★★ 여기가 수정된 부분입니다! ★★★

	// 2. '공용' 스킬 목록 대신, 맵(TMap)에서 'EnemyType'에 해당하는 '상세 정보'를 가져옵니다.
	const FEnemyClassDefaultInfo& Info = EnemyInfo->GetClassDefaultInfo(EnemyType);

	// 3. '상세 정보(Info)' 안에 있는 '고유 스킬(StartupAbilities)' 목록을 순회하며 부여합니다.
	for (TSubclassOf<UGameplayAbility> AbilityClass : Info.StartupAbilities)
	{
		// 4. 어빌리티 레벨을 캐릭터의 실제 레벨로 설정하기 위해, 캐릭터를 가져옵니다.
		ICombatInterface* CombatInterface = Cast<ICombatInterface>(ASC->GetAvatarActor());
		if (CombatInterface == nullptr) continue; // 만약 캐릭터가 아니라면 이 스킬은 건너뜁니다.
		
		//int32 CharacterLevel = CombatInterface->GetPlayerLevel(); // GetPlayerLevel() 함수는 인터페이스에 있어야 합니다.

		// 5. 캐릭터 레벨을 사용하여 어빌리티 스펙을 만들고 부여합니다.
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
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

void USAbilityFunctionLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
	const FVector& SphereOrigin)
{
	OutOverlappingActors.Empty();

	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_Pawn);

	World->OverlapMultiByObjectType(
		Overlaps, 
		SphereOrigin, 
		FQuat::Identity, 
		ObjectQueryParams, 
		FCollisionShape::MakeSphere(Radius), 
		SphereParams
	);

	for (const FOverlapResult& OverlapResult : Overlaps)
	{
		AActor* OverlappedActor = OverlapResult.GetActor();
		
		if (!IsValid(OverlappedActor))
		{
			continue;
		}
		
		ICombatInterface* CombatInterface = Cast<ICombatInterface>(OverlappedActor);
		
		if (CombatInterface && !CombatInterface->IsDead())
		{
			AActor* Avatar = CombatInterface->GetAvatar();
			
			if (IsValid(Avatar))
			{
				OutOverlappingActors.AddUnique(Avatar);
			}
		}
	}
}