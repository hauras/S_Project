#include "Actor/Projectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Actor/FrostField.h" 


AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; 

	bReplicates = true;
	AActor::SetReplicateMovement(true);
	InitialLifeSpan = 5.0f;

	bAlwaysRelevant = true; // 서버가 무조건 클라에게 정보를 보냄
	NetPriority = 3.0f;
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>("CollisionComponent");
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

	// [복구] 꼬리 컴포넌트 생성
	TrailParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>("TrailParticleComponent");
	TrailParticleComponent->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->InitialSpeed = 1500.f;
	ProjectileMovementComponent->MaxSpeed = 4000.f; 
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// [수정] 원본 투사체도 생성 직후 아주 찰나의 순간 동안은 충돌을 끕니다.
	// 이 0.05초가 클라이언트 화면에 이펙트를 '등장'시키는 귀중한 시간이 됩니다.
	if (CollisionComponent && !bIsShard)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 0.05초 뒤에 충돌을 다시 켭니다.
		FTimerHandle CollisionTimer;
		GetWorld()->GetTimerManager().SetTimer(CollisionTimer, [this]()
		{
			if (CollisionComponent) 
				CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}, 0.05f, false);
	}

	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnSphereOverlap);
	}
}

void AProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bIsShard) return;
	if (IsValid(HomingTarget))
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), HomingTarget->GetActorLocation(), FColor::Green, false, 0.1f, 0, 5.0f);
	}
	OrbitData.TimeElapsed += DeltaSeconds;

	// 1. 회전 로직 (삼각함수로 적 주변 뱅글 돌리기)
	if (OrbitData.bIsOrbiting)
	{
		float Rad = FMath::DegreesToRadians(OrbitData.TimeElapsed * 500.f); 
		FVector NewLoc = OrbitData.Center + FVector(FMath::Cos(Rad) * 200.f, FMath::Sin(Rad) * 200.f, 100.f);
		SetActorLocation(NewLoc);

		if (OrbitData.TimeElapsed >= OrbitData.HomingDelay)
		{
			OrbitData.bIsOrbiting = false;
			if (CollisionComponent) CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			
			if (IsValid(HomingTarget))
			{
				ProjectileMovementComponent->bIsHomingProjectile = true;
				ProjectileMovementComponent->HomingAccelerationMagnitude = 20000.f;
				ProjectileMovementComponent->HomingTargetComponent = HomingTarget->GetRootComponent();
				ProjectileMovementComponent->Velocity = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 1500.f;
			}
		}
	}
}

void AProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	// 1. 기초 필터링 (나, 소환사, 유효하지 않은 액터)
	if (OtherActor == GetInstigator() || OtherActor == this || OtherActor == nullptr) return;

	/** 
	 * [핵심 수정] 장판 무시 로직
	 * 만약 부딪힌 대상이 장판(AFrostField)이라면, 파괴하지 않고 그냥 함수를 종료합니다. 
	 * 이렇게 해야 투사체가 장판을 통과하면서 속도만 느려집니다.
	 */
	if (OtherActor->IsA(AFrostField::StaticClass()))
	{
		return;
	}

	// 2. 조각(Shard)이 회전(Orbiting) 중일 때는 다른 충돌 무시
	if (bIsShard && OrbitData.bIsOrbiting) return;

	// 3. 데미지 및 분열 판정
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
	{
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
		FGameplayEffectContextHandle Context = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
		
		// 데미지 적용
		TargetASC->ApplyGameplayEffectSpecToSelf(*TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context).Data.Get());

		// 시너지 분열 체크 (원본일 때만)
		if (!bIsShard && TargetASC->HasMatchingGameplayTag(TargetTag))
		{
			Shatter(OtherActor); 
		}
	}

	// 4. [폭발 연출] 적이나 벽에 부딪혀서 파괴될 때만 실행
	if (ImpactCueTag.IsValid())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator()))
		{
			FGameplayCueParameters Params;
			Params.Location = GetActorLocation();
			ASC->ExecuteGameplayCue(ImpactCueTag, Params);
		}
	}
	
	// 5. 최종 파괴
	Destroy(); 
}

void AProjectile::Shatter(AActor* HitTarget)
{
	if (!HitTarget || !ProjectileClass) return;

	TArray<AActor*> OutActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	TArray<AActor*> IgnoreActors = { this, GetInstigator() };
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), 1000.f, ObjectTypes, ACharacter::StaticClass(), IgnoreActors, OutActors);

	if (OutActors.Num() == 0) OutActors.Add(HitTarget);

	for (int32 i = 0; i < 3; ++i)
	{
		FVector SpawnLocation = HitTarget->GetActorLocation() + FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), 150.f);

		if (AProjectile* Shard = GetWorld()->SpawnActorDeferred<AProjectile>(ProjectileClass, FTransform(GetActorRotation(), SpawnLocation), GetOwner(), GetInstigator()))
		{
			Shard->bIsShard = true;
			Shard->SetActorScale3D(FVector(5.0f)); 
            
			// [추가] 중요 태그/이펙트 데이터 전달 확인
			Shard->DamageEffectClass = DamageEffectClass; 
			Shard->TargetTag = TargetTag;
			Shard->ImpactCueTag = ImpactCueTag;
			Shard->HomingTarget = OutActors[i % OutActors.Num()];

			Shard->OrbitData.bIsOrbiting = true;
			Shard->OrbitData.Center = HitTarget->GetActorLocation();

			// [안정화] 콤포넌트가 확실히 활성화되도록 강제 호출
			if (Shard->TrailParticleComponent)
			{
				Shard->TrailParticleComponent->SetTemplate(TrailParticleComponent->Template); // 에셋 전달
				Shard->TrailParticleComponent->Activate(true);
			}

			if (Shard->CollisionComponent) Shard->CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			Shard->SetActorTickEnabled(true);
			Shard->FinishSpawning(FTransform(GetActorRotation(), SpawnLocation));
		}
	}
}