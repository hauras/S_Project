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

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters Params;
		Params.Location = PlayerCharacter->GetActorLocation(); // 출발 지점
		Params.EffectCauser = PlayerCharacter;
		ASC->AddGameplayCue(TeleportCueTag, Params);
	}

	GetWorld()->GetTimerManager().SetTimer(TrailTimerHandle, this, &UTeleportBase::SpawnTrail, 0.03f, true);

	UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = false; // 대쉬 중 회전 고정
		MoveComp->GroundFriction = 0.f; // 마찰력 제거로 매끄럽게 이동
	}
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
					Character->GetMesh(),         
					NAME_None,                    
					FVector::ZeroVector,
					FRotator::ZeroRotator,
					EAttachLocation::SnapToTarget,
					true                         
				);

			if (NiagaraComp)
			{
				NiagaraComp->SetNiagaraVariableObject(TEXT("MyMesh"),Character->GetMesh());
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
	GetWorld()->GetTimerManager().ClearTimer(TrailTimerHandle);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveGameplayCue(TeleportCueTag); 
	}

	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true; // 회전 복구
		Character->GetCharacterMovement()->GroundFriction = 8.f; 
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}