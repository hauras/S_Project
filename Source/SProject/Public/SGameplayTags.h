
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

	FGameplayTag Ability_BasicAttack;
private:
	static FSGameplayTags GameplayTags;
};
