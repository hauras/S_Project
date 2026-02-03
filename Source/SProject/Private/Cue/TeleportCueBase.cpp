#include "Cue/TeleportCueBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ATeleportCueBase::ATeleportCueBase()
{
	bAutoDestroyOnRemove = true;
	PrimaryActorTick.bCanEverTick = false; 
}

bool ATeleportCueBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (StartEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), StartEffect, Parameters.Location);
	}

	ACharacter* TargetChar = Cast<ACharacter>(MyTarget);
	if (TargetChar && DashAuraEffect)
	{
		ActiveLoopComponent = UGameplayStatics::SpawnEmitterAttached(
			DashAuraEffect, 
			TargetChar->GetMesh(), 
			AttachSocketName, 
			FVector::ZeroVector, 
			FRotator::ZeroRotator, 
			EAttachLocation::SnapToTarget
		);
	}

	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool ATeleportCueBase::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (EndEffect && MyTarget)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EndEffect, MyTarget->GetActorLocation());
	}

	if (ActiveLoopComponent)
	{
		ActiveLoopComponent->Deactivate();
		ActiveLoopComponent = nullptr;
	}

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}