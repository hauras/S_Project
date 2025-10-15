
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"

class UInputMappingContext;
class USInputConfig;
class USInputComponent;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class SPROJECT_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ASPlayerController();
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USInputConfig> InputConfig;

	void Move_Input(const FInputActionValue& InputAction);
	void Look_Input(const FInputActionValue& InputAction);

	UPROPERTY()
	TObjectPtr<USInputComponent> MyInputComponent;
};
