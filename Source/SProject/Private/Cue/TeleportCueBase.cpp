#include "Cue/TeleportCueBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ATeleportCueBase::ATeleportCueBase()
{
	bAutoDestroyOnRemove = true;
	PrimaryActorTick.bCanEverTick = false; // 틱은 필요 없음
}

bool ATeleportCueBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// 1. 출발지 폭발 이펙트 (사라진 자리에 남겨둠)
	if (StartEffect)
	{
		// C++에서 보낸 Parameters.Location(출발지)을 사용
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), StartEffect, Parameters.Location);
	}

	// 2. 이동 중 몸싸개 오라 (캐릭터에 부착)
	ACharacter* TargetChar = Cast<ACharacter>(MyTarget);
	if (TargetChar && DashAuraEffect)
	{
		// 캐릭터 몸(Mesh)의 골반(pelvis) 소켓에 붙여서 같이 날아감
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
	// 3. 도착지 폭발 이펙트 (현재 캐릭터 발밑)
	if (EndEffect && MyTarget)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EndEffect, MyTarget->GetActorLocation());
	}

	// 4. 이동 중이던 오라 제거
	if (ActiveLoopComponent)
	{
		ActiveLoopComponent->Deactivate();
		ActiveLoopComponent = nullptr;
	}

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}