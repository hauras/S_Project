
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "SPlayerState.generated.h"

class UInventoryComponent;
class UPlayerDataAsset;
class UAbilitySystemComponent;
class UAttributeSet;
/**
 * 
 */
UCLASS()
class SPROJECT_API ASPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ASPlayerState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UPROPERTY(EditAnywhere, Replicated)
	TArray<TObjectPtr<UPlayerDataAsset>> PlayerData;

	UPROPERTY(Replicated)
	int32 CurrentCharacterIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentCharacterTag)
	FGameplayTag CurrentCharacterTag;

	UFUNCTION()
	void OnRep_CurrentCharacterTag(FGameplayTag OldTag);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> Inventory;
protected:
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	
};
