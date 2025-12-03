

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

	
	GameplayTags.Ability_Skill_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.BasicAttack"), FString("BasicAttack"));
	GameplayTags.Ability_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.HitReact"), FString("HitReact"));
	
	GameplayTags.Ability_Skill_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.Dodge"), FString("Dodge"));
	GameplayTags.Cooldown_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Dodge"), FString("Cooldown_Dodge"));

	GameplayTags.Ability_Skill_FrostAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.FrostAttack"), FString("FrostAttack"));
	GameplayTags.Cooldown_FrostAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.FrostAttack"), FString("Cooldown_FrostAttack"));

	GameplayTags.Ability_Skill_IceProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Skill.IceProjectile"), FString("IceProjectile"));
	GameplayTags.Cooldown_IceProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.IceProjectile"), FString("Cooldown_IceProjectile"));

	GameplayTags.Combat_Socket_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Combat.Socket.Weapon"), FString("Combat_Socket_Weapon"));


	
}
