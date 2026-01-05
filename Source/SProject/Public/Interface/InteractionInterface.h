
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SPROJECT_API IInteractionInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(AActor* InInteractor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowInteractionWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideInteractionWidget();
};
