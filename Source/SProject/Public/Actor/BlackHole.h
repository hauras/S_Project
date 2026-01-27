
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlackHole.generated.h"

class UGameplayEffect;
class USphereComponent;

UCLASS()
class SPROJECT_API ABlackHole : public AActor
{
	GENERATED_BODY()
	
public:	
	ABlackHole(); 


protected:

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	TObjectPtr<UParticleSystemComponent> ParticleComp;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	TObjectPtr<UParticleSystem> ParticleSystem;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	TObjectPtr<UParticleSystem> ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	TSubclassOf<UGameplayEffect> TickDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	TSubclassOf<UGameplayEffect> ExplosionDamageEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	float PullPower = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackhole")
	float LifeTime = 3.f;
	
	FTimerHandle PullTimerHandle;
	FTimerHandle ExplodeTimerHandle;
	
	virtual void BeginPlay() override;

	void PullEnemy();
	void Explode();
};
