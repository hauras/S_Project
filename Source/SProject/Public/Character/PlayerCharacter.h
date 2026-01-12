
#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Interface/InteractionInterface.h"
#include "PlayerCharacter.generated.h"

class UInventoryComponent;
class USInputConfig;
class USpringArmComponent;
class UCameraComponent;


/**
 * 
 */
UCLASS()
class SPROJECT_API APlayerCharacter : public ACharacterBase, public IInteractionInterface
{
	GENERATED_BODY()
public:
	APlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	virtual void Tick(float DeltaTime) override;

	AActor* GetTarget() const { return Target; }
	virtual void InitAbilityActorInfo() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FGameplayTag CharacterTag;
	
protected:

	UPROPERTY()
	TObjectPtr<AActor> Target;

	void PerformInteractionTrace();

	void LoadProgressFromGameInstance(); 

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	
	
};
