
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

struct FSGameplayTags
{
public:
	static const FSGameplayTags& Get() { return GameplayTags;}
	static void InitializeNativeGameplayTags();

	FGameplayTag InputTag_Move;
	FGameplayTag InputTag_Look;
	FGameplayTag InputTag_BasicAttack;
	FGameplayTag InputTag_Dodge;
	FGameplayTag InputTag_FrostAttack;
	FGameplayTag InputTag_IceProjectile;
	
	FGameplayTag Vital_Health;
	FGameplayTag Vital_Mana;
	
	FGameplayTag Ability_BasicAttack;
	FGameplayTag Ability_HitReact;

	FGameplayTag Ability_Dodge;
	FGameplayTag Cooldown_Dodge;

	FGameplayTag Ability_FrostAttack;
	FGameplayTag Cooldown_FrostAttack;

	FGameplayTag Ability_IceProjectile;
	FGameplayTag Cooldown_IceProjectile;

	FGameplayTag Combat_Socket_Weapon;
	
private:
	static FSGameplayTags GameplayTags;
};
