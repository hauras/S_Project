

#include "SGameplayTags.h"

FSGameplayTags FSGameplayTags::GameplayTags;

void FSGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.InputTag_Move = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Move"), FString("Move"));
	GameplayTags.InputTag_Look = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Look"), FString("Look"));
	GameplayTags.InputTag_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.BasicAttack"), FString("Input_Attack"));
	GameplayTags.InputTag_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Dodge"), FString("InputTag_Dodge"));
	GameplayTags.InputTag_FrostAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.FrostAttack"), FString("InputTag_FrostAttack"));
	GameplayTags.InputTag_IceProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.IceProjectile"), FString("InputTag_IceProjectile"));

	
	GameplayTags.Ability_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.BasicAttack"), FString("BasicAttack"));
	GameplayTags.Ability_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.HitReact"), FString("HitReact"));
	
	GameplayTags.Ability_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Dodge"), FString("Dodge"));
	GameplayTags.Cooldown_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Dodge"), FString("Cooldown_Dodge"));

	GameplayTags.Ability_FrostAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.FrostAttack"), FString("FrostAttack"));
	GameplayTags.Cooldown_FrostAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.FrostAttack"), FString("Cooldown_FrostAttack"));

	GameplayTags.Ability_IceProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.IceProjectile"), FString("IceProjectile"));
	GameplayTags.Cooldown_IceProjectile = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.IceProjectile"), FString("Cooldown_IceProjectile"));

	GameplayTags.Combat_Socket_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Combat.Socket.Weapon"), FString("Combat_Socket_Weapon"));


	
}
