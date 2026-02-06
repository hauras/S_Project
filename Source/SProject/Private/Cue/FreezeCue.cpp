

#include "Cue/FreezeCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

AFreezeCue::AFreezeCue()
{
	IceParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("IceParticle"));
	SetRootComponent(IceParticle);
	bAutoDestroyOnRemove = true;

	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Freeze"));

}

bool AFreezeCue::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	ACharacter* Target = Cast<ACharacter>(MyTarget);
	if (Target && Target->GetMesh())
	{
		// 1. [애니메이션 정지]
		Target->GetMesh()->GlobalAnimRateScale = 0.0f;

		// 2. [부착] 캡슐 중앙(배꼽 위치)에 딱 붙입니다.
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::KeepWorld, // 스케일은 내가 정한 값을 유지
			false
		);
		AttachToComponent(Target->GetRootComponent(), AttachRules);
		
		SetActorRelativeLocation(FVector(0.f, 0.f, 0.f)); 
        
		// 크기를 5~8배로 확실하게 키웁니다.
		IceParticle->SetRelativeScale3D(FVector(2.0f)); 
		
		IceParticle->Activate(true);
	}
	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AFreezeCue::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	ACharacter* Target = Cast<ACharacter>(MyTarget);
	if (Target)
	{
		Target->GetMesh()->GlobalAnimRateScale = 1.0f;

		Target->GetMesh()->SetOverlayMaterial(nullptr);

		if (ShatterParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShatterParticle, Target->GetActorLocation());
		}
	}
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}
