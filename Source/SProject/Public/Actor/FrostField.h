#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "FrostField.generated.h"

class USphereComponent;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FFrostFieldData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float LifeTime = 5.f;
	
	UPROPERTY(EditAnywhere)
	float ProjectileSlowRate = 0.2f;

	UPROPERTY(EditAnywhere)
	float TickInterval = 0.5f;
	
};

UCLASS()
class SPROJECT_API AFrostField : public AActor
{
	GENERATED_BODY()
	
public:	
	AFrostField();
	
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	TSubclassOf<UGameplayEffect> FreezeEffectClass;

	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	TSubclassOf<UGameplayEffect> ExplosionEffectClass;
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UParticleSystemComponent> FieldParticle;
	
	UPROPERTY(EditAnywhere, Category = "Ability")
	FFrostFieldData Data;

	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag FieldCueTag; 

	void OnFieldTick();
	void FinalExplosion();
	FTimerHandle FieldTickTimerHandle;

	UFUNCTION()
	void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:
	
	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, float> AffectedProjectiles;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, int32> EnemyFrostStacks;

	const int32 MaxFrostStacks = 5;

	
};