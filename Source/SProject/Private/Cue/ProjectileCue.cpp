

#include "Cue/ProjectileCue.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AProjectileCue::AProjectileCue()
{
	TrailParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailParticle"));
	SetRootComponent(TrailParticle);
	bAutoDestroyOnRemove = true;

}


bool AProjectileCue::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// EffectCauser는 우리가 서버에서 넣어준 Projectile(this)입니다.
	AActor* ProjectileActor = Parameters.EffectCauser.Get();

	if (ProjectileActor && TrailParticle)
	{
		// 큐 액터를 투사체에 부착
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::SnapToTarget, 
			EAttachmentRule::KeepWorld,    
			false
		);

		this->AttachToComponent(ProjectileActor->GetRootComponent(), AttachRules);
        
		TrailParticle->Activate(true);
	}
    
	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AProjectileCue::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// 투사체가 파괴되어 RemoveGameplayCue가 호출되면 일로 들어옵니다.
	if (TrailParticle)
	{
		TrailParticle->Deactivate();
	}
    
	// bAutoDestroyOnRemove = true 설정 덕분에 잠시 후 이 액터는 자동으로 삭제됩니다.
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}