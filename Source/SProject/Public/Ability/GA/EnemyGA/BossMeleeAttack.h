
#pragma once

#include "CoreMinimal.h"
#include "Ability/GA/EnemyGA/EnemyMeleeAttack.h"
#include "BossMeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API UBossMeleeAttack : public UEnemyMeleeAttack
{
	GENERATED_BODY()
public:
	virtual void HandleDamageEvent(FGameplayEventData Payload);

protected:

	UPROPERTY(EditDefaultsOnly)
	float DamageRadius = 210.f;
};
