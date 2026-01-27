

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
			DrawDebugSphere(GetWorld(), GetActorLocation(), SphereComp->GetScaledSphereRadius(), 32, FColor::Blue, false, 0.1f);

			Enemy->LaunchCharacter(Velocity, true, true);

			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy))
			{
				// 가해자(마법사)의 ASC를 가져옴
				UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());

				// 투사체와 100% 동일한 Context 생성
				FGameplayEffectContextHandle EffectContext = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);

				FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(TickDamageEffectClass, 1.f, EffectContext);
				
				if (SpecHandle.IsValid())
				{
					// 태그 없이 바로 적용 (에셋에 설정된 값을 따름)
					TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Detected Actor: %s"), *Actor->GetName());

	}

}

void ABlackHole::Explode()
{
	// 1. 타이머 중지
	GetWorld()->GetTimerManager().ClearTimer(PullTimerHandle);

	// 2. 주변 적들 다시 찾기 (폭발 범위)
	TArray<AActor*> OverlappingActors;
	SphereComp->GetOverlappingActors(OverlappingActors);

	// 3. GAS 방식으로 한 명씩 데미지 봉투(GE) 전달
	for (AActor* Actor : OverlappingActors)
	{
		ACharacter* Enemy = Cast<ACharacter>(Actor);
		if (Enemy && Enemy != GetOwner())
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy))
			{
				UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
				FGameplayEffectContextHandle Context = SourceASC ? SourceASC->MakeEffectContext() : TargetASC->MakeEffectContext();

				// [중요] 이번엔 '폭발용 GE'를 사용합니다.
				FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(ExplosionDamageEffectClass, 1.f, Context);
                
				if (SpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}

	// 4. 이펙트 및 파괴
	if (ExplosionEffect) 
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	Destroy();
}

