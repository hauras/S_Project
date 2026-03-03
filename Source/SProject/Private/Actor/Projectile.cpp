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
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

AProjectile::AProjectile()
{
    // 최적화: Tick을 완전히 끕니다.
    PrimaryActorTick.bCanEverTick = false; 
    bReplicates = true;
    AActor::SetReplicateMovement(true); 

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
    ProjectileMovementComponent->MaxSpeed = 3000.f;
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnSphereOverlap);
    }

    // Shard(분열 탄환)일 때만 타이머 가동
    if (bIsShard)
    {
        // 1. 충돌 유예 타이머: 0.2초 후에 충돌을 켭니다. (적 몸 뚫고 나오기 위함)
        FTimerHandle CollisionTimer;
        GetWorldTimerManager().SetTimer(CollisionTimer, [this]()
        {
            if (CollisionComponent) 
            {
                CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
        }, OrbitData.CollisionDelay, false);

        // 2. 유도 시작 타이머: 0.5초 후에 유도로 전환합니다.
        FTimerHandle HomingTimer;
        GetWorldTimerManager().SetTimer(HomingTimer, this, &AProjectile::ActivateHoming, OrbitData.HomingDelay, false);

        // 3. 타겟 감시 타이머: 0.1초마다 타겟의 사망 여부를 체크합니다. (Tick 대용)
        FTimerHandle TargetCheckTimer;
        GetWorldTimerManager().SetTimer(TargetCheckTimer, this, &AProjectile::ManageTargetStatus, 0.1f, true);
    }
}

void AProjectile::ActivateHoming()
{
    OrbitData.bIsOrbiting = false;

    // 서버 권한으로 유도 타겟 컴포넌트 설정
    if (HasAuthority() && IsValid(HomingTarget) && ProjectileMovementComponent)
    {
        ProjectileMovementComponent->bIsHomingProjectile = true;
        ProjectileMovementComponent->HomingAccelerationMagnitude = 18000.f;
        ProjectileMovementComponent->HomingTargetComponent = HomingTarget->GetRootComponent();
    }
}

void AProjectile::ManageTargetStatus()
{
    if (!HasAuthority()) return; 

    // 유도 단계이고 현재 타겟이 유효할 때만 사망 체크
    if (!OrbitData.bIsOrbiting && IsValid(HomingTarget))
    {
        ICombatInterface* CombatInt = Cast<ICombatInterface>(HomingTarget);
        if (CombatInt && CombatInt->IsDead())
        {
            // 타겟이 죽었으므로 주변 1000유닛 내 새로운 타겟 검색
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
                // 타겟이 없으면 유도 중단
                HomingTarget = nullptr;
                ProjectileMovementComponent->bIsHomingProjectile = false;
            }
        }
    }
}

void AProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 이펙트 재생 (멀티플레이어 모든 클라이언트)
    if (ImpactVFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactVFX, GetActorLocation());
    }

    // 데미지 및 소멸 로직은 서버에서만 처리
    if (!HasAuthority()) return; 

    if (OtherActor == GetInstigator() || OtherActor == this || OtherActor == nullptr) return;

    ICombatInterface* CombatInterface = Cast<ICombatInterface>(OtherActor);
    if (CombatInterface && !CombatInterface->IsDead())
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
            FGameplayEffectContextHandle Context = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
            FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
            TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            // 첫 번째 탄환이 맞았을 때만 분열(Shatter) 호출
            if (!bIsShard && TargetASC->HasMatchingGameplayTag(TargetTag))
            {
                Shatter(OtherActor); 
            }
        }
    }

    Destroy();
}

void AProjectile::Shatter(AActor* HitTarget)
{
    if (!HasAuthority() || !HitTarget || !ProjectileClass) return;

    // 주변 타겟 검색
    TArray<AActor*> OutActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    TArray<AActor*> IgnoreActors = { this, GetInstigator(), HitTarget };
    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), HitTarget->GetActorLocation(), 1200.f, ObjectTypes, ACharacter::StaticClass(), IgnoreActors, OutActors);

    if (OutActors.Num() == 0) OutActors.Add(HitTarget);

    const int32 ShardCount = 3;
    
    // [수정] 생성 위치를 더 멀게 설정 (200 -> 450)
    const float RingRadius = 450.f; 
    // [수정] 높이도 약간 조절하여 더 웅장하게 배치 (300 -> 400)
    const float RingHeight = 400.f; 

    for (int32 i = 0; i < ShardCount; ++i)
    {
        float Angle = i * (360.f / ShardCount);
        float Rad = FMath::DegreesToRadians(Angle);

        // 적 위치 기준 원형 배치
        FVector Offset = FVector(FMath::Cos(Rad) * RingRadius, FMath::Sin(Rad) * RingRadius, RingHeight);
        FVector SpawnLocation = HitTarget->GetActorLocation() + Offset;
        
        // 생성 시 일단 적을 바라보게 설정
        FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, HitTarget->GetActorLocation());
        FTransform SpawnTransform(SpawnRotation, SpawnLocation);

        AProjectile* Shard = GetWorld()->SpawnActorDeferred<AProjectile>(
            ProjectileClass, SpawnTransform, GetOwner(), GetInstigator(), 
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn 
        );

        if (Shard)
        {
            // [추가] 조각의 크기를 줄임 (기존 크기의 0.4배)
            // 0.4f 값을 조절하여 원하는 크기로 맞추세요.
            Shard->SetActorScale3D(FVector(1.3f, 1.3f, 1.3f));

            Shard->bIsShard = true;                     
            Shard->DamageEffectClass = DamageEffectClass; 
            Shard->TargetTag = TargetTag;
            Shard->HomingTarget = OutActors[i % OutActors.Num()];

            // 3초 대기 설정
            Shard->OrbitData.CollisionDelay = 0.5f; 
            Shard->OrbitData.HomingDelay = 1.0f; 

            if (Shard->CollisionComponent)
            {
                Shard->CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }

            Shard->OrbitData.bIsOrbiting = true;

            // 소환 직후엔 멈춰 있게 함
            if (Shard->ProjectileMovementComponent)
            {
                Shard->ProjectileMovementComponent->Velocity = FVector::ZeroVector;
            }

            Shard->SetLifeSpan(8.0f);
            Shard->FinishSpawning(SpawnTransform);
        }
    }
}