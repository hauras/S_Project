

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
		// 1. [위치 보정] Parameters.Location이 유효하지 않으면 적의 위치를 사용합니다.
		FVector SpawnLoc = Parameters.Location;
		if (SpawnLoc.IsNearlyZero() && MyTarget)
		{
			SpawnLoc = MyTarget->GetActorLocation();
		}

		SpawnLoc += FVector(0.f, 0.f, 100.f);

		// [디버그 로그] 이펙트가 소환되는 좌표를 출력 (0,0,0 인지 확인용)
		UE_LOG(LogTemp, Warning, TEXT("Synergy VFX Spawning at: %s"), *SpawnLoc.ToString());

		// 3. 이펙트 소환 (Cascade용)
		if (ExplosionFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, SpawnLoc, FRotator::ZeroRotator, ExplosionScale);
			
			// 화면 메시지로 성공 알림
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Synergy VFX SUCCESS!"));
		}

		// 4. 사운드 재생
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, SpawnLoc);
		}
	}
}