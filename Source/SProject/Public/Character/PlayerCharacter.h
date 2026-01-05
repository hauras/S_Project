
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
protected:

	UPROPERTY()
	TObjectPtr<AActor> Target;

	void PerformInteractionTrace();
private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> Inventory;
	
	virtual void InitAbilityActorInfo() override;
};
