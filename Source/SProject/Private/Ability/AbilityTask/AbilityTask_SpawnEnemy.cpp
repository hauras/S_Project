

#include "Ability/AbilityTask/AbilityTask_SpawnEnemy.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/EnemyCharacter.h" 
#include "NavigationSystem.h"

UAbilityTask_SpawnEnemy* UAbilityTask_SpawnEnemy::SpawnEnemies(UGameplayAbility* OwningAbility, FGameplayTag EventTag,
                                                               TSoftClassPtr<AEnemyCharacter> EnemyClassToSpawn, int32 NumToSpawn, const FVector& SpawnOrigin,
                                                               float RandomSpawnRadius, const FRotator& SpawnRotator)
{
	UAbilityTask_SpawnEnemy* Node = NewAbilityTask<UAbilityTask_SpawnEnemy>(OwningAbility);
	Node->CachedSpawnOrigin = SpawnOrigin;
	Node->CachedEventTag = EventTag;
	Node->CachedRandomSpawnRadius = RandomSpawnRadius;
	Node->CachedSpawnRotator = SpawnRotator;
	Node->CachedEnemyClassToSpawn = EnemyClassToSpawn; 
	Node->CachedNumToSpawn = NumToSpawn;
	return Node;
}

void UAbilityTask_SpawnEnemy::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("SpawnEnemy Task Activated!"));

	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);	

	DelegateHandle = Delegate.AddUObject(this, &ThisClass::OnGameplayEventReceived);
}

void UAbilityTask_SpawnEnemy::OnDestroy(bool bInOwnerFinished)
{
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);	
	Delegate.Remove(DelegateHandle);

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_SpawnEnemy::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	UClass* LoadedClass = CachedEnemyClassToSpawn.LoadSynchronous();
	
	// 안전장치: 로드 실패나 방송 불가 상태면 종료
	if (!LoadedClass || !ShouldBroadcastAbilityTaskDelegates()) 
	{
		EndTask();
		return;
	}

	TArray<AEnemyCharacter*> SpawnedList;
	UWorld* World = GetWorld();
	
	// 내비게이션 시스템 가져오기
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

	for (int32 i = 0; i < CachedNumToSpawn; i++)
	{
		FVector FinalLocation = CachedSpawnOrigin; // 기본값은 보스 위치

		// ★ 강의 방식: 실제 갈 수 있는 랜덤 위치 찾기
		if (NavSys)
		{
			FNavLocation NavLocation;
			if (NavSys->GetRandomReachablePointInRadius(CachedSpawnOrigin, CachedRandomSpawnRadius, NavLocation))
			{
				FinalLocation = NavLocation.Location;
			}
		}

		// 공중에 살짝 띄워서 소환 (충돌 방지용)
		FinalLocation.Z += 100.f; 

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AEnemyCharacter* NewEnemy = World->SpawnActor<AEnemyCharacter>(LoadedClass, FinalLocation, CachedSpawnRotator, Params))
		{
			// ★ 소환 직후 강제로 떨어지게 만들기 (공중 부양 방지)
			SpawnedList.Add(NewEnemy);
		}
	}

	if (SpawnedList.Num() > 0) OnSpawnFinished.Broadcast(SpawnedList);
	EndTask();
}