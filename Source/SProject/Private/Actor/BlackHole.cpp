

#include "Actor/BlackHole.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"

ABlackHole::ABlackHole()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SetRootComponent(SphereComp); 

	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Overlap);

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>("ParticleComp");
	ParticleComp->SetupAttachment(RootComponent);

	bReplicates = true;

	// 움직이는 액터(투사체 등)라면 위치값 복제도 켜줘야 합니다.
	AActor::SetReplicateMovement(true); 
}
void ABlackHole::BeginPlay()
{
	Super::BeginPlay();

	if (ParticleSystem)
	{
		ParticleComp->SetTemplate(ParticleSystem);
		ParticleComp->Activate();
	}

	GetWorld()->GetTimerManager().SetTimer(
		PullTimerHandle,
		this,
		&ABlackHole::PullEnemy,
		0.1f,
		true
		);

	GetWorld()->GetTimerManager().SetTimer(
	ExplodeTimerHandle, 
	this, 
	&ABlackHole::Explode, 
	LifeTime, 
	false
	);
	
}

void ABlackHole::PullEnemy()
{
	TArray<AActor*> OverlappingActors;
	SphereComp->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		ACharacter* Enemy = Cast<ACharacter>(Actor);
		if (Enemy && Enemy != GetOwner())
		{
			FVector Direction  = GetActorLocation() - Enemy->GetActorLocation();
			Direction = Direction.GetSafeNormal();
			Direction.Z += 0.5f;
			Direction = Direction.GetSafeNormal();

			FVector Velocity = Direction * PullPower;

			Enemy->LaunchCharacter(Velocity, true, true);

			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy))
			{
				UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());

				FGameplayEffectContextHandle EffectContext = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);

				FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(TickDamageEffectClass, 1.f, EffectContext);
				
				if (SpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}

	}

}

void ABlackHole::Explode()
{
	// 1. 타이머 중지
	GetWorld()->GetTimerManager().ClearTimer(PullTimerHandle);

	// 2. 주변 적들 다시 찾기 (폭발 범위)
	TArray<AActor*> OverlappingActors;
	SphereComp->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		ACharacter* Enemy = Cast<ACharacter>(Actor);
		if (Enemy && Enemy != GetOwner())
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy))
			{
				UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
				FGameplayEffectContextHandle Context = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();

				FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(ExplosionDamageEffectClass, 1.f, Context);
                
				if (SpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}

	if (ExplosionEffect) 
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	Destroy();
}

