#include "Actor/FrostField.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

AFrostField::AFrostField()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true; 

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);

	FieldParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FieldParticle"));
	FieldParticle->SetupAttachment(RootComponent);
	
	// 콜리전 설정
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void AFrostField::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// 0.5초마다 적을 감지하고 스택을 쌓는 타이머 시작
		GetWorldTimerManager().SetTimer(FieldTickTimerHandle, this, &AFrostField::OnFieldTick, Data.TickInterval, true);
		
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &AFrostField::OnDetectionBeginOverlap);
		Sphere->OnComponentEndOverlap.AddDynamic(this, &AFrostField::OnDetectionEndOverlap);

		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator()))
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = GetActorLocation();
			CueParams.EffectCauser = this;
			ASC->AddGameplayCue(FieldCueTag, CueParams);
		}

		SetLifeSpan(Data.LifeTime);
	}
}

void AFrostField::Destroyed()
{
	if (HasAuthority())
	{
		// 1. 감속되었던 투사체들 복구 
		for (auto& It : AffectedProjectiles)
		{
			if (AActor* Projectile = It.Key.Get()) 
			{
				if (UProjectileMovementComponent* MoveComp = Projectile->FindComponentByClass<UProjectileMovementComponent>())
				{
					MoveComp->MaxSpeed = It.Value;
					MoveComp->Velocity = MoveComp->Velocity.GetSafeNormal() * MoveComp->MaxSpeed;
				}
			}
		}

		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator()))
		{
			ASC->RemoveGameplayCue(FieldCueTag);
		}

		FinalExplosion();
	}
	Super::Destroyed();
}

void AFrostField::OnFieldTick()
{
	if (!HasAuthority()) return;

	TArray<AActor*> OverlappingActors;
	Sphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == GetInstigator() || !IsValid(Actor)) continue;

		// 1. 적별로 스택 계산
		int32& CurrentStack = EnemyFrostStacks.FindOrAdd(Actor);
		CurrentStack++;
		
		// 2. [빙결 판정] 5스택 도달 시
		if (CurrentStack == MaxFrostStacks)
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
			{
				if (FreezeEffectClass)
				{
					FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(FreezeEffectClass, 1.f, TargetASC->MakeEffectContext());
					if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
					{
						TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
		}
	}
}

void AFrostField::FinalExplosion()
{
	if (!HasAuthority()) return;

	TArray<AActor*> OutActors;
	Sphere->GetOverlappingActors(OutActors, ACharacter::StaticClass());

	for (AActor* Actor : OutActors)
	{
		if (Actor == GetInstigator() || !IsValid(Actor)) continue;

		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
		{
			FGameplayTag FreezeTag = FGameplayTag::RequestGameplayTag(FName("State.Stun.Freeze"));
			
			if (TargetASC->HasMatchingGameplayTag(FreezeTag))
			{
				
				TargetASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(FreezeTag));

				// 추가로 강력한 파쇄 데미지만 입혀주면 끝!
				FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
				Context.AddInstigator(GetInstigator(), this);
				FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(ExplosionEffectClass, 1.f, Context);
				if (SpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}
}

void AFrostField::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	// 내 총알은 무시
	if (GetInstigator() == OtherActor->GetInstigator()) return;
	
	// 적 투사체 불렛 타임 적용
	if (UProjectileMovementComponent* ProjMove = OtherActor->FindComponentByClass<UProjectileMovementComponent>())
	{
		if (!AffectedProjectiles.Contains(OtherActor))
		{
			AffectedProjectiles.Add(OtherActor, ProjMove->MaxSpeed);
			ProjMove->MaxSpeed *= Data.ProjectileSlowRate;
			ProjMove->Velocity *= Data.ProjectileSlowRate;
			
			// 장판 안에서 사라지지 않게 수명 무제한 설정
			OtherActor->SetLifeSpan(0.f); 
		}
	}
}

void AFrostField::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	// 투사체가 나가면 속도 복구
	if (AffectedProjectiles.Contains(OtherActor))
	{
		if (UProjectileMovementComponent* ProjMove = OtherActor->FindComponentByClass<UProjectileMovementComponent>())
		{
			if (AffectedProjectiles.Contains(OtherActor))
			{
				float OrigSpeed = AffectedProjectiles[OtherActor];
				ProjMove->MaxSpeed = OrigSpeed;
				ProjMove->Velocity = ProjMove->Velocity.GetSafeNormal() * OrigSpeed;
			}
		}
		AffectedProjectiles.Remove(OtherActor);
	}

	// 캐릭터가 장판을 나가면 스택 초기화
	if (EnemyFrostStacks.Contains(OtherActor))
	{
		EnemyFrostStacks.Remove(OtherActor);
	}
}