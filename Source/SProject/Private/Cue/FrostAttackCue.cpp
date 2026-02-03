#include "Cue/FrostAttackCue.h"
#include "Kismet/GameplayStatics.h"

UFrostAttackCue::UFrostAttackCue()
{
	// 1. 이 큐가 반응할 태그를 지정합니다. (에디터의 태그와 일치해야 함)
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Skill.FrostAttack"));
}

bool UFrostAttackCue::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	// 부모 로직 실행
	Super::OnExecute_Implementation(MyTarget, Parameters);

	// 2. 위치 결정 (전달받은 Location이 없으면 타겟의 위치 사용)
	FVector SpawnLoc = Parameters.Location.IsNearlyZero() ? (MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector) : Parameters.Location;

	// 3. 지면에서 살짝 띄우기 (이펙트가 바닥에 파묻히지 않게 함)
	SpawnLoc += FVector(0.f, 0.f, 20.f);

	// 4. 이펙트 소환
	if (FrostParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FrostParticle, SpawnLoc, FRotator::ZeroRotator, EffectScale);
	}

	// 5. 사운드 재생
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, SpawnLoc);
	}

	return true;
}