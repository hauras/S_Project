

#include "Cue/MarkBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h" // 추가

AMarkBase::AMarkBase()
{
	MarkParticle = CreateDefaultSubobject<UParticleSystemComponent>("MarkParticle");
	SetRootComponent(MarkParticle);
	
	// 루프 이펙트이므로 자동 파괴는 꺼둠 (직접 제어)
	bAutoDestroyOnRemove = true;
}

bool AMarkBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	ACharacter* TargetCharacter = Cast<ACharacter>(MyTarget);
	if (TargetCharacter && MarkParticle)
	{
		// 위치와 회전은 소켓에 맞추되(Snap), 스케일은 블루프린트 설정을 유지(KeepRelative)합니다.
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget, // Location
			EAttachmentRule::SnapToTarget, // Rotation
			EAttachmentRule::KeepRelative, // Scale <--- 이 부분이 핵심!
			true                           // bWeldSimulatedBodies
		);
        
		AttachToComponent(TargetCharacter->GetRootComponent(), AttachRules);

		// 높이 보정 로직 (기존 코드)
		float CapsuleHalfHeight = TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		MarkParticle->SetRelativeLocation(FVector(0.f, 0.f, CapsuleHalfHeight + 10.f));
        

		MarkParticle->Activate(true);
	}
	return true;
}

bool AMarkBase::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (MarkParticle)
	{
		MarkParticle->Deactivate();
	}
	
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}
