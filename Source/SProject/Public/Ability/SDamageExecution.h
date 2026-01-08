
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "SDamageExecution.generated.h"


/**
 * 
 */
UCLASS()
class SPROJECT_API USDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	USDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
