

#include "SGameplayTags.h"

FSGameplayTags FSGameplayTags::GameplayTags;

void FSGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.InputTag_Move = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Move"), FString("Move"));
	GameplayTags.InputTag_Look = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Look"), FString("Look"));
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.LMB"), FString("InputTag_LMB"));
	GameplayTags.InputTag_Space = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Space"), FString("InputTag_Space"));
	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.1"), FString("InputTag_1"));
	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.2"), FString("InputTag_2"));
	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.3"), FString("InputTag_3"));

	GameplayTags.InputTag_Interact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Interact"), FString("Interact"));
	GameplayTags.InputTag_Inventory = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Inventory"), FString("Inventory"));
	GameplayTags.InputTag_Tag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Tag"), FString("Tag"));

	GameplayTags.Ability_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.HitReact"), FString("HitReact"));

	// 전사 스킬
	GameplayTags.Ability_Skill_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.BasicAttack"), FString("BasicAttack"));
	
	GameplayTags.Ability_Skill_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.Dodge"), FString("Dodge"));
	GameplayTags.Cooldown_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Dodge"), FString("Cooldown_Dodge"));

	GameplayTags.Ability_Skill_FrostAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.FrostAttack"), FString("FrostAttack"));
	GameplayTags.Cooldown_FrostAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.FrostAttack"), FString("Cooldown_FrostAttack"));

	GameplayTags.Ability_Skill_IceProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.IceProjectile"), FString("IceProjectile"));
	GameplayTags.Cooldown_IceProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.IceProjectile"), FString("Cooldown_IceProjectile"));

	// 마법사 스킬
	GameplayTags.Ability_Skill_MageAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.MageAttack"), FString("MageAttack"));

	GameplayTags.Ability_Skill_Flare = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.Flare"), FString("Flare"));
	GameplayTags.Cooldown_Flare = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Flare"), FString("Cooldown_Flare"));

	GameplayTags.Ability_Skill_Beam = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.Beam"), FString("Beam"));
	GameplayTags.Cooldown_Beam = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Beam"), FString("Cooldown_Beam"));

	GameplayTags.Ability_Skill_Teleport = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.Teleport"), FString("Teleport"));
	GameplayTags.Cooldown_Teleport = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Teleport"), FString("Cooldown_Teleport"));

	GameplayTags.Ability_Skill_BlackHole = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.BlackHole"), FString("BlackHole"));
	GameplayTags.Cooldown_BlackHole = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.BlackHole"), FString("Cooldown_BlackHole"));

	// 공격 부위
	GameplayTags.Combat_Socket_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Combat.Socket.Weapon"), FString("Combat_Socket_Weapon"));
	GameplayTags.Combat_Socket_Fist_Right = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Combat.Socket.Fist.Right"), FString("Combat_Socket_Fist_Right"));
	GameplayTags.Combat_Socket_Fist_Left = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Combat.Socket.Fist.Left"), FString("Combat_Socket_Fist_Left"));
	
	GameplayTags.Event_Montage_Attack_Fist_Right = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Event.Montage.Attack.Fist.Right"), FString("Event_Montage_Attack_Fist_Right"));
	GameplayTags.Event_Montage_Attack_Fist_Left = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Event.Montage.Attack.Fist.Left"), FString("Event_Montage_Attack_Fist_Left"));
	
	GameplayTags.Ability_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Attack"), FString("Attack"));

	GameplayTags.Ability_Attack_Boss_Melee1 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Attack.Boss.Melee1"), FString("Ability_Attack_Boss_Melee1"));
	GameplayTags.Ability_Attack_Boss_Melee2 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Attack.Boss.Melee2"), FString("Ability_Attack_Boss_Melee2"));
	GameplayTags.Ability_Attack_Boss_Melee3 = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Attack.Boss.Melee3"), FString("Ability_Attack_Boss_Melee3"));

	GameplayTags.Ability_Summon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Summon"), FString("Summon"));
	GameplayTags.Event_Montage_Summon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Event.Montage.Summon"), FString("Montage_Summon"));

	GameplayTags.State_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("State.Stun"), FString("Stun"));
	GameplayTags.State_Mark_Warrior = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("State.Mark.Warrior"), FString("Mark.Warrior"));
	GameplayTags.State_Mark_Mage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("State.Mark.Mage"), FString("Mark.Mage"));
	GameplayTags.Ability_Synergy_Explosion = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("State.Synergy.Explosion"), FString("Synergy.Explosion"));

	GameplayTags.Character_Warrior = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Character.Warrior"), FString("Warrior"));
	GameplayTags.Character_Mage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Character.Mage"), FString("Mage"));

}
