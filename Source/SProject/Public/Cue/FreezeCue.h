
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "FreezeCue.generated.h"

class UParticleSystemComponent;
class UParticleSystem;
/**
 * 
 */
UCLASS()
class SPROJECT_API AFreezeCue : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
public:
	AFreezeCue();

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 빙결 종료 시 (스턴 시간 만료 시)
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UParticleSystemComponent> IceParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UParticleSystem> ShatterParticle;

	
};
