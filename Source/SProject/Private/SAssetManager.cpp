

#include "SAssetManager.h"
#include "SGameplayTags.h"
USAssetManager& USAssetManager::Get()
{
	check(GEngine);

	USAssetManager* SAssetManager = Cast<USAssetManager>(GEngine->AssetManager);
	return *SAssetManager;
}

void USAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FSGameplayTags::InitializeNativeGameplayTags();
}
