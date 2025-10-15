
#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "SAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API USAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:

	static USAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
