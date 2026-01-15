

#include "Actor/Projectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Interface/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("CollisionComponent");
	SetRootComponent(CollisionComponent);

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>("ParticleComponent");
	ParticleComponent->SetupAttachment(RootComponent);
 
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 날아가는 방향으로 자동 회전
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;

	ProjectileMovementComponent->InitialSpeed = 1500.f;
	ProjectileMovementComponent->MaxSpeed = 1500.f;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnSphereOverlap);
}

void AProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	// 1. [안전 장치] 나 자신과 나를 쏜 주인(Pawn)은 무시합니다.
	if (OtherActor == GetInstigator() || OtherActor == this || OtherActor == nullptr) return;

	// 2. 데미지 적용 로직 (캐릭터인 경우)
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(OtherActor);
	if (CombatInterface && !CombatInterface->IsDead())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// ★ 중요: Context는 맞는 놈(Target)이 아니라 쏜 놈(Instigator)에게서 만들어야 
			// 쏜 놈의 '공격력' 스탯을 계산기가 읽어갈 수 있습니다! ⭐
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
			
			FGameplayEffectContextHandle EffectContext = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContext);
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// 3. [공통 피드백] 무엇에 맞았든(벽이든 적이든) 연출을 보여주고 소멸시킵니다.
	if (ImpactVFX)
	{
		// 이펙트 소환
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactVFX, GetActorLocation());
	}
	
	// 소리도 추가하면 디테일이 살겠죠? (선택사항)
	// if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(...);

	Destroy(); // 이제 벽에 맞아도 사라집니다!
}


