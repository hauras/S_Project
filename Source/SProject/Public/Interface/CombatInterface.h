
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SPROJECT_API ICombatInterface
{
	GENERATED_BODY()

public:
	//virtual void AttackHitCheck() = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& Target);
	
	virtual void Die() = 0;
	virtual bool IsDead() const = 0;

	virtual AActor* GetAvatar() = 0;
	
	virtual FVector GetSocketLocation(const FGameplayTag& SocketTag) const = 0;
	virtual FName GetCombatSocketNameFromTag(const FGameplayTag& SocketTag) const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsBoss() const;
};
