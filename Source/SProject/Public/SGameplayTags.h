
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
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_Space;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	
	FGameplayTag Vital_Health;
	FGameplayTag Vital_Mana;
	
	FGameplayTag Ability_Skill_BasicAttack;
	FGameplayTag Ability_HitReact;

	FGameplayTag Ability_Skill_Dodge;
	FGameplayTag Cooldown_Dodge;

	FGameplayTag Ability_Skill_FrostAttack;
	FGameplayTag Cooldown_FrostAttack;

	FGameplayTag Ability_Skill_IceProjectile;
	FGameplayTag Cooldown_IceProjectile;

	FGameplayTag Combat_Socket_Weapon;
	
private:
	static FSGameplayTags GameplayTags;
};
