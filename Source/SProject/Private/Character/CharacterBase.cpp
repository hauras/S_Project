

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

AActor* ACharacterBase::GetAvatar() 
{
	return this;
}

FVector ACharacterBase::GetSocketLocation(const FGameplayTag& SocketTag) const
{
	FName SocketName = GetCombatSocketNameFromTag(SocketTag); // 위에서 만든 함수 호출
	if (SocketName != NAME_None)
	{
		return GetMesh()->GetSocketLocation(SocketName);
	}
	return GetActorLocation();
}

FName ACharacterBase::GetCombatSocketNameFromTag(const FGameplayTag& SocketTag) const
{
	const FSGameplayTags& GameplayTags  = FSGameplayTags::Get();
	if (SocketTag.MatchesTagExact(GameplayTags.Combat_Socket_Weapon)) return FName("WeaponEndSocket");
	if (SocketTag.MatchesTagExact(GameplayTags.Combat_Socket_Fist_Right)) return FName("Fist_R_Socket");
	if (SocketTag.MatchesTagExact(GameplayTags.Combat_Socket_Fist_Left)) return FName("Fist_L_Socket");

	return NAME_None;
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

	if (RegenEffectClass)
	{
		ApplyEffectToSelf(RegenEffectClass, 1.f);
	}
}

void ACharacterBase::InitAbilityActorInfo()
{
	
}

void ACharacterBase::Die()
{
	// 중복된 죽음을 방지하는 가드는 그대로 둡니다.
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;
	
	// 서버에서만 멀티캐스트를 호출합니다.
	if (HasAuthority())
	{
		MulticastHandleDeath();
	}
}


void ACharacterBase::MulticastHandleDeath_Implementation()
{
	// 1. AI 로직 중지 (안전하게 체크)
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (AIController->BrainComponent) // ← 반드시 체크해야 합니다!
		{
			AIController->BrainComponent->StopLogic("Dead");
		}
	}

	// 2. 캐릭터의 이동 및 충돌 비활성화 (Getter 사용 및 유효성 검사)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 3. 죽음 애니메이션 몽타주 재생
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}


