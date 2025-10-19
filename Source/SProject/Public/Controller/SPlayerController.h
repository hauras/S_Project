
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "SPlayerController.generated.h"

class UInputAction;
class USAbilitySystemComponent;
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

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY()
	TObjectPtr<USInputComponent> MyInputComponent;

	UPROPERTY()
	TObjectPtr<USAbilitySystemComponent> SAbilitySystemComponent;
	
	USAbilitySystemComponent* GetASC();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
};
