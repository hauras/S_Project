

#include "Character/CharacterBase.h"

#include "AIController.h"
#include "SGameplayTags.h"
#include "Ability/SAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BrainComponent.h"


ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ACharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

bool ACharacterBase::IsDead() const
{
	return bIsDead;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACharacterBase::AddCharacterAbilities()
{
	USAbilitySystemComponent* SASC = CastChecked<USAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;

	SASC->AddCharacterAbilities(StartupAbilities);
}

void ACharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	//check(GameplayEffectClass);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ACharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(MaxVitalAttributes, 1.f);
	ApplyEffectToSelf(VitalAttributes, 1.f);

}

void ACharacterBase::InitAbilityActorInfo()
{
	
}

void ACharacterBase::Die()
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;
	
	if (HasAuthority())
	{
		MulticastHandleDeath();
	}
}

void ACharacterBase::MulticastHandleDeath_Implementation()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->BrainComponent->StopLogic("Dead");
	}

	// 2. 캐릭터의 이동 및 충돌을 비활성화합니다.
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3. 죽음 애니메이션 몽타주를 재생합니다.
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}


