#include "Cue/BeamBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Interface/CombatInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

ABeamBase::ABeamBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bAutoDestroyOnRemove = true;
	BeamPSC = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BeamPSC"));
	SetRootComponent(BeamPSC);
	BeamPSC->bAutoActivate = false; 
}

bool ABeamBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	OwnerActor = Parameters.EffectCauser.Get();
	if (OwnerActor)
	{
		AttachedSocketName = FName("Fist_L_Socket"); 

		if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
		{

			AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, AttachedSocketName);
		}
		BeamPSC->Activate(true);
	}
	return true;
}

void ABeamBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (OwnerActor && BeamPSC)
	{
		// 1. 위치 업데이트
		FVector Start = GetActorLocation(); 
		if (const ACharacter* Character = Cast<ACharacter>(OwnerActor))
		{
			Start = Character->GetMesh()->GetSocketLocation(AttachedSocketName);
		}
		
		FVector End = GetBeamEndLocation(); 
		FVector Dir = End - Start;

		SetActorLocation(Start);

		FRotator TargetRot = Dir.Rotation();
		SetActorRotation(TargetRot);
		
		BeamPSC->SetRelativeRotation(FRotator(-90.f, -90.f, 0.f)); 

		float Distance = Dir.Size();
		BeamPSC->SetWorldScale3D(FVector(Distance / 100.f, 2.0f, 2.0f));
	}
}

FVector ABeamBase::GetBeamEndLocation() const
{
	if (!OwnerActor) return FVector::ZeroVector;
	APawn* Pawn = Cast<APawn>(OwnerActor);
	if (!Pawn || !Pawn->GetController()) return OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 2000.f;

	FVector ViewLocation; FRotator ViewRotation;
	Pawn->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FRotator FixedRotation = ViewRotation;
	FixedRotation.Pitch = FMath::ClampAngle(FixedRotation.Pitch, -45.0f, 10.0f);

	FVector TraceStart = ViewLocation + (FixedRotation.Vector() * 100.f);
	FVector TraceEnd = TraceStart + (FixedRotation.Vector() * 5000.f);
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);

	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		return Hit.Location;
	}

	return TraceEnd;
} 
bool ABeamBase::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (BeamPSC) BeamPSC->Deactivate();
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}