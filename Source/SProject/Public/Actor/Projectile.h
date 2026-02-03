#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent; // 추가
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FProjectileOrbitData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector Center = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsOrbiting = false;   

	UPROPERTY(VisibleAnywhere)
	float TimeElapsed = 0.f;

	float CollisionDelay = 0.2f; 
	float HomingDelay = 0.5f;    
};

UCLASS()
class SPROJECT_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// [수정] 폭발은 여전히 큐를 사용합니다 (최적화)
	UPROPERTY(EditAnywhere, Category = "Projectile|Visual")
	FGameplayTag ImpactCueTag;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FGameplayTag TargetTag;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	// [복구] 꼬리 이펙트는 액터 내장 컴포넌트로 사용해야 안정적입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystemComponent> TrailParticleComponent;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsShard = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FProjectileOrbitData OrbitData;

	UPROPERTY()
	TObjectPtr<AActor> HomingTarget;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Shatter(AActor* HitTarget);
};