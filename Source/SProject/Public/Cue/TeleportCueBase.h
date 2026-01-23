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

	// 시전 시 (출발 + 오라 부착)
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 제거 시 (도착 이펙트)
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	// --- 에셋 설정 (블루프린트 디테일창에서 선택 가능) ---
	
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> StartEffect; // P_PhaseLinkActivate (출발)

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> DashAuraEffect; // P_LinkPullAura (이동 중)

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> EndEffect; // P_PhaseLinkImpact (도착)

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName AttachSocketName = FName("pelvis");

private:
	// 이동 중인 오라를 나중에 지우기 위해 저장해두는 컴포넌트
	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ActiveLoopComponent;
};