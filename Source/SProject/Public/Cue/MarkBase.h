
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "MarkBase.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API AMarkBase : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
public:
	AMarkBase();

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UParticleSystemComponent> MarkParticle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FName AttachSocketName = FName("head");

	// 머리 위로 약 30~50cm 띄우기 위해 Z값 설정 (X는 0으로)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FVector LocationOffset = FVector(0.0f, 0.0f, 40.0f);
};
