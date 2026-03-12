
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "SPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterIndexChanged, int32 /*NewIndex*/);

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

	FOnCharacterIndexChanged OnCharacterIndexChangedDelegate;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentCharacterIndex, BlueprintReadOnly, Category = "Player Data")
	int32 CurrentCharacterIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentCharacterTag)
	FGameplayTag CurrentCharacterTag;

	UFUNCTION()
	void OnRep_CurrentCharacterTag(FGameplayTag OldTag);

	UFUNCTION()
	void OnRep_CurrentCharacterIndex(int32 OldIndex);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	bool bAttributesInitialized = false;
	
protected:
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	
};
