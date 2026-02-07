#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h" // 태그 사용을 위해 필수
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class UGameplayEffect;

// 1. 회전(Orbit) 데이터를 관리하는 구조체
USTRUCT(BlueprintType)
struct FOrbitData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsOrbiting = false;     // 현재 회전 중인가?

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Center = FVector::ZeroVector; // 회전 중심점 (적의 위치)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 200.f;         // 회전 반경

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 500.f;          // 회전 속도 (초당 도)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TimeElapsed = 0.f;      // 회전 경과 시간

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float InitialAngle = 0.f;     // 스폰 시 시작 각도

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

	// 외부(GA)에서 넘겨줄 데이터
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FGameplayTag TargetTag;

protected:
	virtual void BeginPlay() override;

	// 컴포넌트들
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UParticleSystemComponent> ParticleComponent;

	// 이펙트 에셋
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TObjectPtr<UParticleSystem> ImpactVFX;

	// --- [분열 및 회전 시스템 변수] ---

	// 분열 시 소환할 클래스 (블루프린트에서 BP_Projectile 자기 자신을 할당)
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile> ProjectileClass;

	// 분열된 조각인지 판별 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	bool bIsShard = false;

	// 회전 관련 데이터 뭉치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	FOrbitData OrbitData;

	// 현재 추적 중인 타겟
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<AActor> HomingTarget;

	// 충돌 시 호출될 함수
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 분열 로직
	void Shatter(AActor* HitTarget);
};