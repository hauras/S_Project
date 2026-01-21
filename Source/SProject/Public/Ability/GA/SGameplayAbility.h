
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SGameplayAbility.generated.h"


/**
 * 
 */
UCLASS()
class SPROJECT_API USGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

protected:
	TArray<FHitResult> GetHitResultFromTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, bool bDrawDebug = false, bool bIgnoreSelf = true) const;

	
};
