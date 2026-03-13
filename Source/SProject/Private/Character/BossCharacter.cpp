#include "Character/BossCharacter.h"
#include "Components/TimelineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"

ABossCharacter::ABossCharacter()
{
	HitTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("HitTimeline"));
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh() && GetMesh()->GetMaterial(0))
	{
		BodyMID = GetMesh()->CreateDynamicMaterialInstance(0, GetMesh()->GetMaterial(0));
	}

	if (HitCurve)
	{
		FOnTimelineFloat HitUpdateFunc;
		HitUpdateFunc.BindUFunction(this, FName("UpdateHitFxSwitch"));
		HitTimeline->AddInterpFloat(HitCurve, HitUpdateFunc);
	}

	if (DissolveCurve)
	{
		FOnTimelineFloat DissolveUpdateFunc;
		DissolveUpdateFunc.BindUFunction(this, FName("UpdateDeathDissolve"));
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveUpdateFunc);

		FOnTimelineEvent FinishedFunc;
		FinishedFunc.BindUFunction(this, FName("OnDissolveTimelineFinished"));
		DissolveTimeline->SetTimelineFinishedFunc(FinishedFunc);
	}
}

void ABossCharacter::PlayHitReactEffect()
{
	if (HitTimeline)
	{
		HitTimeline->PlayFromStart();
	}
}

void ABossCharacter::Die()
{
	Super::Die();

	SetLifeSpan(0.0f); 
}

void ABossCharacter::MulticastHandleDeath_Implementation()
{
	Super::MulticastHandleDeath_Implementation();

	const float DeathAnimDuration = 3.1f; 

	FTimerHandle DeathEffectTimerHandle;
	GetWorldTimerManager().SetTimer(DeathEffectTimerHandle, [this]()
	{
		if (GetMesh())
		{
			GetMesh()->bPauseAnims = true;
		}

		if (DeathNiagaraSystem)
		{
			DeathNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				DeathNiagaraSystem,
				GetMesh(),
				NAME_None,
				FVector::ZeroVector, 
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true
			);
		}

		if (DissolveTimeline)
		{
			DissolveTimeline->PlayFromStart();
		}

	}, DeathAnimDuration, false);
} 

void ABossCharacter::UpdateHitFxSwitch(float HitFxValue)
{
	if (BodyMID)
	{
		BodyMID->SetScalarParameterValue(FName("HitFxSwitch"), HitFxValue);
	}
}

void ABossCharacter::UpdateDeathDissolve(float DissolveValue)
{
	if (BodyMID)
	{
		BodyMID->SetScalarParameterValue(FName("DissolveAmount"), DissolveValue);
	}

	if (DeathNiagaraComp)
	{
		DeathNiagaraComp->SetNiagaraVariableFloat(TEXT("User.DissolveAmount"), DissolveValue);
	}
}

void ABossCharacter::OnDissolveTimelineFinished()
{
	Destroy();
}