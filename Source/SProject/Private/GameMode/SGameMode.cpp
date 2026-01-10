

#include "GameMode/SGameMode.h"

#include "Character/EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"

void ASGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), FoundEnemies);

	int32 ValidEnemyCount = 0;
	// ★ 여기서 for문을 돌려 '살아있는' 놈들만 셉니다.
	for (AActor* EnemyActor : FoundEnemies)
	{
		ICombatInterface* CombatInt = Cast<ICombatInterface>(EnemyActor);
		if (CombatInt && !CombatInt->IsDead())
		{
			ValidEnemyCount++;
		}
	}
	EnemyCount = ValidEnemyCount;

	UE_LOG(LogTemp, Warning, TEXT("게임 시작! 잡아야 할 적: %d명"), EnemyCount);
}

void ASGameMode::OnEnemyKilled()
{
	// 1. 숫자를 하나 줄입니다.
	EnemyCount--;

	UE_LOG(LogTemp, Warning, TEXT("적 처치! 남은 적: %d명"), EnemyCount);

	if (EnemyCount <= 0)
	{
		
	}
}
