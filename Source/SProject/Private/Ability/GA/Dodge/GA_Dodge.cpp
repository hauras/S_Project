#include "Ability/GA/Dodge/GA_Dodge.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h" 

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

    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;
    }
    
    const FVector DodgeDirection = Character->GetActorForwardVector() * -1.0f;

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