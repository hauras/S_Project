
#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API AEnemyCharacter : public ACharacterBase
{
	GENERATED_BODY()
public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;
};
