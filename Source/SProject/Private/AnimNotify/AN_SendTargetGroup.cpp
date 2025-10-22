
#include "AnimNotify/AN_SendTargetGroup.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

UAN_SendTargetGroup::UAN_SendTargetGroup()
{
	StartSocketName = TEXT("weaponstartSocket");
	EndSocketName = TEXT("weaponEndSocket");
	TraceRadius = 25.0f;
	EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Damage"));
}

void UAN_SendTargetGroup::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner() && MeshComp->GetOwner()->HasAuthority())
	{
		// 노티파이 구간이 시작될 때, 중복 피격 방지 배열을 깨끗하게 비웁니다.
		AlreadyHitActors.Empty();

		// 첫 프레임의 위치를 '이전 위치'로 저장해 둡니다.
		PreviousStartLocation = MeshComp->GetSocketLocation(StartSocketName);
		PreviousEndLocation = MeshComp->GetSocketLocation(EndSocketName);
	}
}

void UAN_SendTargetGroup::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner() || !MeshComp->GetOwner()->HasAuthority())
	{
		return;
	}

	// 현재 프레임의 소켓 위치를 가져옵니다.
	const FVector CurrentStartLocation = MeshComp->GetSocketLocation(StartSocketName);
	const FVector CurrentEndLocation = MeshComp->GetSocketLocation(EndSocketName);
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(MeshComp->GetOwner());

	TArray<FHitResult> OutHits;

	// 이전 프레임 위치 -> 현재 프레임 위치로 SphereTrace를 실행하여 '궤적'을 만듭니다.
	// 칼날 전체를 커버하기 위해 시작 소켓과 끝 소켓, 두 번의 트레이스를 실행합니다.
	UKismetSystemLibrary::SphereTraceMulti(MeshComp->GetWorld(), PreviousStartLocation, CurrentStartLocation, TraceRadius, 
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHits, true);

	UKismetSystemLibrary::SphereTraceMulti(MeshComp->GetWorld(), PreviousEndLocation, CurrentEndLocation, TraceRadius, 
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHits, true);

	// 감지된 모든 대상에 대해 처리합니다.
	for (const FHitResult& Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		// 유효한 액터이고, 아직 이번 스윙에 맞은 적이 없다면
		if (HitActor && !AlreadyHitActors.Contains(HitActor))
		{
			AlreadyHitActors.Add(HitActor); // 중복 방지 배열에 추가

			// 어빌리티로 보낼 이벤트 데이터를 만듭니다.
			FGameplayEventData Payload;
			Payload.Target = HitActor; // 가장 중요한 정보: 누가 맞았는가!

			// 어빌리티 시스템에 이벤트를 보냅니다.
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, Payload);
		}
	}

	// 다음 Tick에서 사용하기 위해 현재 위치를 '이전 위치'로 갱신합니다.
	PreviousStartLocation = CurrentStartLocation;
	PreviousEndLocation = CurrentEndLocation;
}
