
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
		AlreadyHitActors.Empty();

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
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn), false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);

	UKismetSystemLibrary::SphereTraceMulti(MeshComp->GetWorld(), PreviousEndLocation, CurrentEndLocation, TraceRadius, 
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn), false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);

	for (const FHitResult& Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !AlreadyHitActors.Contains(HitActor))
		{
			AlreadyHitActors.Add(HitActor); 

			FGameplayEventData Payload;
			Payload.Target = HitActor; 

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, Payload);
		}
	}

	PreviousStartLocation = CurrentStartLocation;
	PreviousEndLocation = CurrentEndLocation;
}
