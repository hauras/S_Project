

#include "Cue/MarkBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h" // 추가

AMarkBase::AMarkBase()
{
	MarkParticle = CreateDefaultSubobject<UParticleSystemComponent>("MarkParticle");
	SetRootComponent(MarkParticle);
	
	bAutoDestroyOnRemove = true;
}

bool AMarkBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	ACharacter* TargetCharacter = Cast<ACharacter>(MyTarget);
	if (TargetCharacter && MarkParticle)
	{
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget, // Location
			EAttachmentRule::SnapToTarget, // Rotation
			EAttachmentRule::KeepRelative, // Scale 
			true                           // bWeldSimulatedBodies
		);
        
		AttachToComponent(TargetCharacter->GetRootComponent(), AttachRules);

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
