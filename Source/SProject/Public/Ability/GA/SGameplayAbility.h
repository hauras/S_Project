
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

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> MarkEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> SynergyEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag MarkTag;

	UFUNCTION(BlueprintCallable, Category = "Synergy")
	void ExecuteSynergyLogic(AActor* Target);
protected:
	TArray<FHitResult> GetHitResultFromTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, bool bDrawDebug = false, bool bIgnoreSelf = true) const;

	
};
