#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "FrostAttackCue.generated.h"

class UParticleSystem;
class USoundBase;

UCLASS()
class SPROJECT_API UFrostAttackCue : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UFrostAttackCue();

	// GameplayCue가 실행될 때 호출되는 핵심 함수
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	// 에디터에서 할당할 이펙트 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> FrostParticle;

	// 에디터에서 할당할 사운드 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX")
	TObjectPtr<USoundBase> ImpactSound;

	// 이펙트 크기 조절
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	FVector EffectScale = FVector(1.0f);
};