#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "TeleportCueBase.generated.h"

class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class SPROJECT_API ATeleportCueBase : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	ATeleportCueBase();

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> StartEffect;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> DashAuraEffect; 

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> EndEffect;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName AttachSocketName = FName("pelvis");

private:
	// 이동 중인 오라를 나중에 지우기 위해 저장해두는 컴포넌트
	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ActiveLoopComponent;
};