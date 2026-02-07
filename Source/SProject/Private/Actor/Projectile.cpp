#include "Actor/Projectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Interface/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("CollisionComponent");
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>("ParticleComponent");
	ParticleComponent->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->InitialSpeed = 1500.f;
	ProjectileMovementComponent->MaxSpeed = 3000.f; // 유도 시 가속을 위해 높게 설정
}
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnSphereOverlap);
	}
}
void AProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsShard) return;

	OrbitData.TimeElapsed += DeltaSeconds;

	// [단계 1: 충돌 유예] 적 몸안에서 생성되어 즉시 터지는 것 방지
	if (OrbitData.TimeElapsed >= OrbitData.CollisionDelay && CollisionComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// [단계 2: 유도 시작 전환]
	if (OrbitData.bIsOrbiting && OrbitData.TimeElapsed >= OrbitData.HomingDelay)
	{
		OrbitData.bIsOrbiting = false; // 확산 단계 종료

		if (IsValid(HomingTarget))
		{
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = 18000.f; // 유도 강도
			ProjectileMovementComponent->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
	}

	// [단계 3: 유도 중 타겟 관리]
	if (!OrbitData.bIsOrbiting && IsValid(HomingTarget))
	{
		ICombatInterface* CombatInt = Cast<ICombatInterface>(HomingTarget);
		if (CombatInt && CombatInt->IsDead())
		{
			// 새 타겟 찾기
			TArray<AActor*> NewOutActors;
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
			TArray<AActor*> IgnoreActors = { this, GetInstigator() };

			if (UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 1000.f, ObjectTypes, ACharacter::StaticClass(), IgnoreActors, NewOutActors))
			{
				HomingTarget = NewOutActors[0];
				ProjectileMovementComponent->HomingTargetComponent = HomingTarget->GetRootComponent();
			}
			else
			{
				HomingTarget = nullptr;
				ProjectileMovementComponent->bIsHomingProjectile = false;
			}
		}
	}
}
void AProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (OtherActor == GetInstigator() || OtherActor == this || OtherActor == nullptr) return;

	if (bIsShard && OrbitData.TimeElapsed < OrbitData.CollisionDelay) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(OtherActor);
	if (CombatInterface && !CombatInterface->IsDead())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
			FGameplayEffectContextHandle Context = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			// 첫 번째 탄환이 맞았을 때만 Shatter 호출
			if (!bIsShard && TargetASC->HasMatchingGameplayTag(TargetTag))
			{
				Shatter(OtherActor); 
			}
		}
	}

	if (ImpactVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactVFX, GetActorLocation());
	}

	Destroy();

}
void AProjectile::Shatter(AActor* HitTarget)
{
	if (!HitTarget || !ProjectileClass) return;

	TArray<AActor*> OutActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TArray<AActor*> IgnoreActors = { this, GetInstigator() };
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 1200.f, ObjectTypes, ACharacter::StaticClass(), IgnoreActors, OutActors);

	if (OutActors.Num() == 0) OutActors.Add(HitTarget);

	for (int32 i = 0; i < 3; ++i)
	{
		float Angle = i * 120.f;
		float Rad = FMath::DegreesToRadians(Angle);
	
		// 튀어나갈 방향 계산 (적 위치 기준 방사형 + 약간 위쪽)
		FVector SpreadDir = FVector(FMath::Cos(Rad), FMath::Sin(Rad), 0.6f).GetSafeNormal();
	
		// 적의 중심에서 살짝 밖에서 생성
		FVector SpawnLocation = GetActorLocation() + (SpreadDir * 30.f);
		FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

		AProjectile* Shard = GetWorld()->SpawnActorDeferred<AProjectile>(
			ProjectileClass, SpawnTransform, GetOwner(), GetInstigator(), 
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn 
		);

		if (Shard)
		{
			Shard->bIsShard = true;                     
			Shard->DamageEffectClass = DamageEffectClass; 
			Shard->TargetTag = TargetTag;
			Shard->HomingTarget = OutActors[i % OutActors.Num()];

			// [중요] 생성 시 충돌을 꺼서 적 몸을 통과하게 함
			if (Shard->CollisionComponent)
			{
				Shard->CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			// 유도탄 설정을 위한 초기화
			Shard->OrbitData.bIsOrbiting = true; // 확산 중임을 표시
			Shard->OrbitData.TimeElapsed = 0.f;
		
			// 밖으로 튀어나가는 초기 속도 부여
			if (Shard->ProjectileMovementComponent)
			{
				Shard->ProjectileMovementComponent->Velocity = SpreadDir * 1000.f;
			}

			Shard->SetActorTickEnabled(true);
			Shard->SetLifeSpan(5.0f);
			Shard->FinishSpawning(SpawnTransform);
		}
	}

}