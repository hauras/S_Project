
#pragma once

#include "CoreMinimal.h"
#include "Character/EnemyCharacter.h"
#include "Components/TimelineComponent.h"
#include "NiagaraSystem.h"
#include "BossCharacter.generated.h"

class UNiagaraComponent;
UCLASS()
class SPROJECT_API ABossCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
public:
	ABossCharacter();
	
	void PlayHitReactEffect();

protected:
	virtual void BeginPlay() override;
	virtual void Die() override;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BodyMID;
	
	UPROPERTY()
	TObjectPtr<UTimelineComponent> HitTimeline;
	
	UPROPERTY(EditAnywhere, Category = "Effects | Hit React")
	TObjectPtr<UCurveFloat> HitCurve;

	UPROPERTY()
	TObjectPtr<UTimelineComponent> DissolveTimeline;
	
	UPROPERTY(EditAnywhere, Category = "Effects | Dissolve")
	TObjectPtr<UCurveFloat> DissolveCurve;

	UFUNCTION()
	void OnDissolveTimelineFinished();

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* DeathNiagaraSystem;

	UPROPERTY()
	UNiagaraComponent* DeathNiagaraComp;

	
	
private:
	UFUNCTION()
	void UpdateHitFxSwitch(float HitFxSwitch);

	UFUNCTION()
	void UpdateDeathDissolve(float DissolveValue);

	virtual void MulticastHandleDeath_Implementation() override;
};