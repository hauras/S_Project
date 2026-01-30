
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "SynergyExplosion.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API USynergyExplosion : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
public:
	USynergyExplosion();

	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UParticleSystem> ExplosionFX; // 만약 나이아가라면 UNiagaraSystem* 사용

	// 2. 폭발 사운드 칸
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<USoundBase> ExplosionSound;

	// 3. 이펙트 크기 조절
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FVector ExplosionScale = FVector(1.0f);
};
