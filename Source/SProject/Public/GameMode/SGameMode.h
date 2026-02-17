
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

class UEnemyStats;
/**
 * 
 */
UCLASS()
class SPROJECT_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly, Category = " Enemy Info")
	TObjectPtr<UEnemyStats> EnemyInfo;


protected:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "Stage Rules")
	int32 EnemyCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Stage Rules")
	TSubclassOf<AActor> PortalClass;

};
