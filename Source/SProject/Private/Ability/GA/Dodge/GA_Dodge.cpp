#include "Ability/GA/Dodge/GA_Dodge.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h" // CharacterMovementComponent를 사용하기 위해 헤더 추가

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacterBase* Character = Cast<ACharacterBase>(ActorInfo->AvatarActor.Get());
    if (!IsValid(Character) || !IsValid(DodgeMontage))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // ★★★ 1. 회피 시작 전, 기존 움직임 설정 변경 (이 부분은 아주 좋습니다!) ★★★
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;
    }
    
    // ★★★ 2. 방향 결정 로직을 '무조건 뒤로' 가도록 수정 ★★★
    // 플레이어의 입력을 확인하는 GetLastMovementInputVector()를 아예 사용하지 않습니다.
    const FVector DodgeDirection = Character->GetActorForwardVector() * -1.0f;

    // ★★★ 3. 회피 속도(거리)는 원하시는 값으로 설정 (5000.f은 매우 빠릅니다!) ★★★
    Character->LaunchCharacter(DodgeDirection.GetSafeNormal() * 5000.f, true, true);

    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("DodgeMontage"), DodgeMontage);
    if (!IsValid(MontageTask))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    MontageTask->OnCompleted.AddDynamic(this, &UGA_Dodge::OnMontageEnded);
    MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dodge::OnMontageEnded);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::OnMontageEnded);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Dodge::OnMontageEnded);

    MontageTask->ReadyForActivation();
}

void UGA_Dodge::OnMontageEnded()
{
    ACharacterBase* Character = Cast<ACharacterBase>(GetCurrentActorInfo()->AvatarActor.Get());
    if (Character)
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            MovementComp->bOrientRotationToMovement = true;
        }
        Character->bUseControllerRotationYaw = false;
    }

    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}