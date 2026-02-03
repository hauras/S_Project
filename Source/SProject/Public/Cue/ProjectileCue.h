
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "ProjectileCue.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API AProjectileCue : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
public:
	AProjectileCue();

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 큐가 끝날 때 (투사체 파괴 시)
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UParticleSystemComponent> TrailParticle;
	
};
