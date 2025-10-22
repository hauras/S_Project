

#include "Character/CharacterBase.h"

#include "SGameplayTags.h"
#include "Ability/SAbilitySystemComponent.h"


ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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
	check(GameplayEffectClass);
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




