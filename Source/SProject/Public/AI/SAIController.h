
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SAIController.generated.h"

class UAISenseConfig_Sight;
class UAIPerceptionComponent;
/**
 * 
 */
UCLASS()
class SPROJECT_API ASAIController : public AAIController
{
	GENERATED_BODY()
public:
	ASAIController();

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
protected:

	UPROPERTY(VisibleAnywhere, Category = "AIPerception" )
	TObjectPtr<UAIPerceptionComponent> EnemyPerceptionComponent;

	UPROPERTY(VisibleAnywhere, Category = "AIPerception" )
	TObjectPtr<UAISenseConfig_Sight> AISenseConfig_Sight;

	UFUNCTION()
	virtual void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	
};

