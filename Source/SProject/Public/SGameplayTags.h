
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
	FGameplayTag Combat_Socket_Fist_Right;
	FGameplayTag Combat_Socket_Fist_Left;

	FGameplayTag Event_Montage_Attack_Fist_Left;
	FGameplayTag Event_Montage_Attack_Fist_Right;
	
	FGameplayTag Ability_Attack;

	FGameplayTag Ability_Attack_Boss_Melee1;
	FGameplayTag Ability_Attack_Boss_Melee2;
	FGameplayTag Ability_Attack_Boss_Melee3;
	
	FGameplayTag Ability_Summon;
	FGameplayTag Event_Montage_Summon;
	
	FGameplayTag State_Stun;
private:
	static FSGameplayTags GameplayTags;
};
