
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
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
class SPROJECT_API ASPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	ASPlayerController();

	virtual FGenericTeamId GetGenericTeamId() const override;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USInputConfig> InputConfig;

	void Move_Input(const FInputActionValue& InputAction);
	void Look_Input(const FInputActionValue& InputAction);
	void Interact_Input(const FInputActionValue& InputAction);
	
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY()
	TObjectPtr<USInputComponent> MyInputComponent;

	UPROPERTY()
	TObjectPtr<USAbilitySystemComponent> SAbilitySystemComponent;
	
	USAbilitySystemComponent* GetASC();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	FGenericTeamId PlayerTeamId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> CrosshairWidgetClassInstance;
};
