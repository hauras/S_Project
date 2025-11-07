

#include "SGameplayTags.h"

FSGameplayTags FSGameplayTags::GameplayTags;

void FSGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.InputTag_Move = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Move"), FString("Move"));
	GameplayTags.InputTag_Look = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Look"), FString("Look"));
	GameplayTags.InputTag_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.BasicAttack"), FString("Input_Attack"));
	GameplayTags.InputTag_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Dodge"), FString("InputTag_Dodge"));

	
	GameplayTags.Ability_BasicAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.BasicAttack"), FString("BasicAttack"));
	GameplayTags.Ability_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.HitReact"), FString("HitReact"));
	GameplayTags.Ability_Dodge = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Dodge"), FString("Dodge"));

}
