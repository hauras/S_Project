#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FOrbitData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsOrbiting = false;     // 현재 확산(Orbit/Spread) 단계인가?

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionDelay = 0.2f;  // 생성 후 충돌이 활성화될 때까지의 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HomingDelay = 0.5f;     // 생성 후 유도로 전환될 때까지의 시간
	
};

UCLASS()
class SPROJECT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FGameplayTag TargetTag;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UParticleSystemComponent> ParticleComponent;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TObjectPtr<UParticleSystem> ImpactVFX;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	bool bIsShard = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	FOrbitData OrbitData;

	// 현재 추적 중인 타겟
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<AActor> HomingTarget;
	
	void ActivateHoming();

	void ManageTargetStatus();

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Shatter(AActor* HitTarget);


};