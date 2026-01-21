#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GameplayTagContainer.h"
#include "BeamBase.generated.h"

class UParticleSystemComponent;

UCLASS()
class SPROJECT_API ABeamBase : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	ABeamBase();

	// 엔진에서 실제로 인식하는 가상 함수 이름들로 복구합니다.
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UParticleSystemComponent> BeamPSC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FName TargetParameterName = FName("Target");

private:
	FName AttachedSocketName;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	FVector GetBeamEndLocation() const;
};