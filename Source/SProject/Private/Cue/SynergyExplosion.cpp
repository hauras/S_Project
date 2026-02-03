

#include "Cue/SynergyExplosion.h"

#include "Kismet/GameplayStatics.h"

USynergyExplosion::USynergyExplosion()
{
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Synergy.Explosion"));

}

void USynergyExplosion::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	if (EventType == EGameplayCueEvent::Executed)
	{
		FVector SpawnLoc = Parameters.Location;
		if (SpawnLoc.IsNearlyZero() && MyTarget)
		{
			SpawnLoc = MyTarget->GetActorLocation();
		}

		SpawnLoc += FVector(0.f, 0.f, 100.f);
		
		// 3. 이펙트 소환 
		if (ExplosionFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, SpawnLoc, FRotator::ZeroRotator, ExplosionScale);
		}

		// 4. 사운드 재생
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, SpawnLoc);
		}
	}
}