

#include "SGameplayTags.h"

FSGameplayTags FSGameplayTags::GameplayTags;

void FSGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.InputTag_Move = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Move"), FString("Move"));
	GameplayTags.InputTag_Look = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("InputTag.Look"), FString("Look"));

}
