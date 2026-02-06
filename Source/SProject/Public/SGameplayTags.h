
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
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_Interact;
	FGameplayTag InputTag_Inventory;
	FGameplayTag InputTag_Tag;
	
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

	FGameplayTag Ability_Skill_FrostField;
	FGameplayTag Cooldown_FrostField;
	
	FGameplayTag Ability_Skill_MageAttack;

	FGameplayTag Ability_Skill_Flare;
	FGameplayTag Cooldown_Flare;

	FGameplayTag Ability_Skill_Beam;
	FGameplayTag Cooldown_Beam;

	FGameplayTag Ability_Skill_Teleport;
	FGameplayTag Cooldown_Teleport;

	FGameplayTag Ability_Skill_BlackHole;
	FGameplayTag Cooldown_BlackHole;
	
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
	FGameplayTag State_Stun_Freeze;
	
	FGameplayTag State_Mark_Warrior;
	FGameplayTag State_Mark_Mage;
	FGameplayTag Ability_Synergy_Explosion;
	
	FGameplayTag Character_Warrior;
	FGameplayTag Character_Mage;

	
private:
	static FSGameplayTags GameplayTags;
};
