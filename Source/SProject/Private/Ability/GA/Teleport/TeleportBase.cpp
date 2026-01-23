#include "Ability/GA/Teleport/TeleportBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h" 
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h" // 상단에 추가 확인
#include "NiagaraComponent.h"

UTeleportBase::UTeleportBase()
{
}

void UTeleportBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) return;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	// 1. 애니메이션 노티파이 대기 (손을 뻗는 순간 실행)
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TeleportTag);
	EventTask->EventReceived.AddDynamic(this, &UTeleportBase::OnTeleportEventReceived);
	EventTask->ReadyForActivation();

	// 2. 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AnimMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UTeleportBase::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UTeleportBase::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UTeleportBase::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void UTeleportBase::OnTeleportEventReceived(FGameplayEventData Payload)
{
	ACharacterBase* PlayerCharacter = Cast<ACharacterBase>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter) return;

	// 3. 이동 방향 결정
	FVector InputDir = PlayerCharacter->GetLastMovementInputVector();
	FVector DashDir = InputDir.IsNearlyZero() ? PlayerCharacter->GetActorForwardVector() : InputDir.GetSafeNormal();

	// 4. 이펙트 실행 (GC_Teleport_Actor 스폰)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters Params;
		Params.Location = PlayerCharacter->GetActorLocation(); // 출발 지점
		Params.EffectCauser = PlayerCharacter;
		ASC->AddGameplayCue(TeleportCueTag, Params);
	}

	// 5. [포폴 핵심] 잔상 타이머 시작 (0.03초마다 현재 위치에 잔상 생성)
	GetWorld()->GetTimerManager().SetTimer(TrailTimerHandle, this, &UTeleportBase::SpawnTrail, 0.03f, true);

	// 6. 물리 발사
	UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = false; // 대쉬 중 회전 고정
		MoveComp->GroundFriction = 0.f; // 마찰력 제거로 매끄럽게 이동
	}
	// 바닥에 박히는 걸 방지하기 위해 Z축으로 힘을 살짝 줌(200.f)
	PlayerCharacter->LaunchCharacter(DashDir * TeleportStrength + FVector(0.f, 0.f, 200.f), true, true);
}

void UTeleportBase::SpawnTrail()
{
	if (TrailParticle)
	{
		AActor* Avatar = GetAvatarActorFromActorInfo();
		ACharacter* Character = Cast<ACharacter>(Avatar);

		if (Character && Character->GetMesh())
		{
			UNiagaraComponent* NiagaraComp =
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					TrailParticle,
					Character->GetMesh(),          // 🔥 스켈레탈 메시
					NAME_None,                     // 소켓 없으면 None
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget,
					true                           // Auto Destroy
				);

			if (NiagaraComp)
			{
				// Niagara User Parameter에 SkeletalMesh 전달
				NiagaraComp->SetNiagaraVariableObject(
					TEXT("MyMesh"),
					Character->GetMesh()
				);

				UE_LOG(LogTemp, Warning, TEXT("Trail Spawned & Mesh Set: %s"),
					*Character->GetMesh()->GetName());
			}
		}
	}
}

void UTeleportBase::OnMontageEnded()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UTeleportBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 7. 종료 시 리소스 정리
	GetWorld()->GetTimerManager().ClearTimer(TrailTimerHandle); // 타이머 끄기

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveGameplayCue(TeleportCueTag); // 이펙트 제거 (GC_Actor의 OnRemove 호출 -> 도착 이펙트 쾅!)
	}

	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true; // 회전 복구
		Character->GetCharacterMovement()->GroundFriction = 8.f; // 마찰력 복구
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}